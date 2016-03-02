#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <memory.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <setjmp.h>
#include "warden.h"


#define INIT_WARDEN()   if (!gFunctions.bInitialized) initWarden();


void* load_symbol(const char* symbol);


/***************************** WARDEN MANAGEMENT ******************************/

typedef struct _tOverloadedFunctions
{
    char bInitialized;

    void* (*malloc)(size_t size);
    void* (*calloc)(size_t count, size_t size);
    void* (*realloc)(void* ptr, size_t size);
    void* (*valloc)(size_t size);
    int (*posix_memalign)(void **memptr, size_t alignment, size_t size);
    void (*free)(void*);

    int (*system)(const char *);
    FILE* (*popen)(const char *, const char *);
    int (*pclose)(FILE *);
    FILE* (*fdopen)(int, const char *);
    FILE* (*fopen)(const char *, const char *);
    FILE* (*freopen)(const char *, const char *, FILE*);
    int (*fclose)(FILE*);
    FILE* (*tmpfile)(void);
    void* (*dlopen)(const char*, int);
    int (*dlclose)(void*);
    int (*socket)(int, int, int);
    int (*close)(int);

    int (*execve)(const char*, char* const [], char* const []);
    int (*execv)(const char*, char* const []);
    int (*execvp)(const char*, char* const []);
    int (*execvP)(const char*, const char*, char* const []);

    void (*exit)(int);
    void (*_Exit)(int);
    void (*_exit)(int);
    void (*atexit)(void (*)(void));
    
    pid_t (*wait)(int*);
    pid_t (*wait3)(int*, int, struct rusage *);
    pid_t (*wait4)(pid_t, int*, int, struct rusage *);
    pid_t (*waitpid)(pid_t, int*, int);
    
    unsigned int (*sleep)(unsigned int);
    int (*usleep)(useconds_t);
    int (*nanosleep)(const struct timespec *, struct timespec *);

    pid_t (*fork)();
    pid_t (*vfork)();

    sig_t (*signal)(int, sig_t);
    int (*sigaction)(int, const struct sigaction *, struct sigaction *);
    int (*sigaltstack)(const stack_t *, stack_t *);
    int (*sigprocmask)(int, const sigset_t *, sigset_t *);
    int (*sigsuspend)(const sigset_t *);
    int (*siginterrupt)(int, int);
    int (*kill)(pid_t, int);

    void (*longjmp)(jmp_buf, int);
    void (*longjmperror)();
    int (*_setjmp)(jmp_buf);
    void (*siglongjmp)(sigjmp_buf, int);
    int (*__sigsetjmp)(sigjmp_buf, int);

} tOverloadedFunctions;


static tWardenListener      gListener   = 0;
static tWardenContext*      gContext    = 0;
static tOverloadedFunctions gFunctions  = { 0 };
static unsigned char        gUnsafeFreeEnabled = 0;
static jmp_buf              gEnvironment;


void initWarden()
{
    #define LOAD_SYMBOL(name)   gFunctions.name = load_symbol(#name)

    gFunctions.bInitialized = 1;
    
    LOAD_SYMBOL(malloc);
    LOAD_SYMBOL(calloc);
    LOAD_SYMBOL(realloc);
    LOAD_SYMBOL(valloc);
    LOAD_SYMBOL(posix_memalign);
    LOAD_SYMBOL(free);

    LOAD_SYMBOL(system);
    LOAD_SYMBOL(popen);
    LOAD_SYMBOL(pclose);
    LOAD_SYMBOL(fdopen);
    LOAD_SYMBOL(fopen);
    LOAD_SYMBOL(freopen);
    LOAD_SYMBOL(fclose);
    LOAD_SYMBOL(tmpfile);
    LOAD_SYMBOL(dlopen);
    LOAD_SYMBOL(dlclose);
    LOAD_SYMBOL(socket);
    LOAD_SYMBOL(close);
    
    LOAD_SYMBOL(execve);
    LOAD_SYMBOL(execv);
    LOAD_SYMBOL(execvp);
    LOAD_SYMBOL(execvP);

    LOAD_SYMBOL(exit);
    LOAD_SYMBOL(_Exit);
    LOAD_SYMBOL(_exit);
    LOAD_SYMBOL(atexit);

    LOAD_SYMBOL(wait);
    LOAD_SYMBOL(wait3);
    LOAD_SYMBOL(wait4);
    LOAD_SYMBOL(waitpid);

    LOAD_SYMBOL(sleep);
    LOAD_SYMBOL(usleep);
    LOAD_SYMBOL(nanosleep);

    LOAD_SYMBOL(fork);
    LOAD_SYMBOL(vfork);

    LOAD_SYMBOL(signal);
    LOAD_SYMBOL(sigaction);
    LOAD_SYMBOL(sigaltstack);
    LOAD_SYMBOL(sigprocmask);
    LOAD_SYMBOL(sigsuspend);
    LOAD_SYMBOL(siginterrupt);
    LOAD_SYMBOL(kill);

    LOAD_SYMBOL(longjmp);
    LOAD_SYMBOL(longjmperror);
    LOAD_SYMBOL(_setjmp);
    LOAD_SYMBOL(siglongjmp);
    LOAD_SYMBOL(__sigsetjmp);
}


void setWardenListener(tWardenListener listener)
{
    gListener = listener;
}


void setWardenContext(tWardenContext* pContext)
{
    gContext = pContext;
}


tWardenContext* getWardenContext()
{
    return gContext;
}


void wardenEnableUnsafeFree()
{
    gUnsafeFreeEnabled = 1;
}


void wardenDisableUnsafeFree()
{
    gUnsafeFreeEnabled = 0;
}


/***************************** UTILITY FUNCTIONS ******************************/

void* load_symbol(const char* symbol)
{
    char* error;

    void* function = dlsym(RTLD_NEXT, symbol);
    if ((error = dlerror()) != NULL)
    {
        (*gListener)(gContext, WARDEN_STATUS_FAILURE, error);
        gContext = 0;
        exit(2);
    }
    
    return function;
}


void free_segv_handler(int sig)
{
    gFunctions.signal(SIGSEGV, SIG_DFL);
    gFunctions.longjmp(gEnvironment, 2);
}


#ifdef MASH_WARDEN_MEMORY_DEBUG
void printMemoryUsage()
{
    unsigned int go = 0;
    unsigned int mo = 0;
    unsigned int ko = 0;
    unsigned int o = 0;

    size_t size = gContext->memory_allocated;
    
    o = size % 1000;
    size = (size - o) / 1000;
    
    if (size > 0)
    {
        ko = size % 1000;
        size = (size - ko) / 1000;
    }

    if (size > 0)
    {
        mo = size % 1000;
        size = (size - mo) / 1000;
    }

    go = size;
    
    if (go > 0)
        printf("Allocated = %d´%03d´%03d´%03do, ", go, mo, ko, o);
    else if (mo > 0)
        printf("Allocated = %d´%03d´%03do, ", mo, ko, o);
    else if (ko > 0)
        printf("Allocated = %d´%03do, ", ko, o);
    else
        printf("Allocated = %do, ", o);

    go = 0;
    mo = 0;
    ko = 0;
    o = 0;

    size = gContext->memory_allocated_maximum;
    
    o = size % 1000;
    size = (size - o) / 1000;
    
    if (size > 0)
    {
        ko = size % 1000;
        size = (size - ko) / 1000;
    }

    if (size > 0)
    {
        mo = size % 1000;
        size = (size - mo) / 1000;
    }

    go = size;
    
    if (go > 0)
        printf("Max allocated = %d´%03d´%03d´%03do\n", go, mo, ko, o);
    else if (mo > 0)
        printf("Max allocated = %d´%03d´%03do\n", mo, ko, o);
    else if (ko > 0)
        printf("Max allocated = %d´%03do\n", ko, o);
    else
        printf("Max allocated = %do\n", o);
}
#endif


/************************* MEMORY-RELATED FUNCTIONS ***************************/

void* malloc(size_t size)
{
    INIT_WARDEN();
        
    if (gContext && (gContext->memory_limit > 0))
    {
        size_t remaining = gContext->memory_limit - gContext->memory_allocated;
        if (size > remaining)
        {
            (*gListener)(gContext, WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, 0);
            gContext = 0;
            exit(2);
        }
        
        gContext->memory_allocated += size;
        
        if (gContext->memory_allocated > gContext->memory_allocated_maximum)
            gContext->memory_allocated_maximum = gContext->memory_allocated;

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printf("[Sandboxed object #%d] malloc(%lu) -> ", gContext->sandboxed_object, (unsigned long) size);
        printMemoryUsage();
#endif

        size_t* ptr = (size_t*) gFunctions.malloc(size + 2 * sizeof(size_t));
        ptr[0] = (size_t) ((char*) (ptr + 2) + size);
        ptr[1] = size;
        
        return (void*) &ptr[2];
    }
    else
    {
        return gFunctions.malloc(size);
    }
}


void* calloc(size_t count, size_t size)
{
    INIT_WARDEN();
        
    if (gContext && (gContext->memory_limit > 0))
    {
        size_t remaining = gContext->memory_limit - gContext->memory_allocated;
        if (count * size > remaining)
        {
            (*gListener)(gContext, WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, 0);
            gContext = 0;
            exit(2);
        }
        
        gContext->memory_allocated += count * size;
        
        if (gContext->memory_allocated > gContext->memory_allocated_maximum)
            gContext->memory_allocated_maximum = gContext->memory_allocated;

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printf("[Sandboxed object #%d] calloc(%lu * %lu) -> ", gContext->sandboxed_object,
               (unsigned long) count, (unsigned long) size);
        printMemoryUsage();
#endif

        size_t* ptr = (size_t*) gFunctions.malloc(count * size + 2 * sizeof(size_t));

        ptr[0] = (size_t) ((char*) (ptr + 2) + count * size);
        ptr[1] = count * size;

        memset(&ptr[2], 0, count * size);
        
        return (void*) &ptr[2];
    }
    else
    {
        return gFunctions.calloc(count, size);
    }
}


void* realloc(void* ptr, size_t size)
{
    INIT_WARDEN();
        
    if (gContext && (gContext->memory_limit > 0))
    {
#ifdef MASH_WARDEN_MEMORY_DEBUG
        size_t before = gContext->memory_allocated;
#endif

        size_t* internal_ptr = 0;
        if (ptr)
        {
            internal_ptr = ((size_t*) ptr) - 2;
            gContext->memory_allocated -= internal_ptr[1];
        }

        size_t remaining = gContext->memory_limit - gContext->memory_allocated;
        if (size > remaining)
        {
            (*gListener)(gContext, WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, 0);
            gContext = 0;
            exit(2);
        }

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printf("[Sandboxed object #%d] realloc(%lu -> %lu) -> ", gContext->sandboxed_object,
               (unsigned long) (before - gContext->memory_allocated), (unsigned long) size);
#endif
        
        gContext->memory_allocated += size;
        
        if (gContext->memory_allocated > gContext->memory_allocated_maximum)
            gContext->memory_allocated_maximum = gContext->memory_allocated;

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printMemoryUsage();
#endif

        size_t* ptr = (size_t*) gFunctions.realloc(internal_ptr, size + 2 * sizeof(size_t));

        ptr[0] = (size_t) ((char*) (ptr + 2) + size);
        ptr[1] = size;

        return (void*) &ptr[2];
    }
    else
    {
        return gFunctions.realloc(ptr, size);
    }
}


void* valloc(size_t size)
{
    INIT_WARDEN();
        
    if (gContext && (gContext->memory_limit > 0))
    {
        size_t remaining = gContext->memory_limit - gContext->memory_allocated;
        if (size > remaining)
        {
            (*gListener)(gContext, WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, 0);
            gContext = 0;
            exit(2);
        }
        
        gContext->memory_allocated += size;
        
        if (gContext->memory_allocated > gContext->memory_allocated_maximum)
            gContext->memory_allocated_maximum = gContext->memory_allocated;

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printf("[Sandboxed object #%d] valloc(%lu) -> ", gContext->sandboxed_object, (unsigned long) size);
        printMemoryUsage();
#endif

        size_t* ptr = (size_t*) gFunctions.valloc(size + 2 * sizeof(size_t));

        ptr[0] = (size_t) ((char*) (ptr + 2) + size);
        ptr[1] = size;

        return (void*) &ptr[2];
    }
    else
    {
        return gFunctions.valloc(size);
    }
}


int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    INIT_WARDEN();

    if (gContext && (gContext->memory_limit > 0))
    {
        size_t remaining = gContext->memory_limit - gContext->memory_allocated;
        if (size > remaining)
        {
            (*gListener)(gContext, WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, 0);
            gContext = 0;
            exit(2);
        }

        gContext->memory_allocated += size;

        if (gContext->memory_allocated > gContext->memory_allocated_maximum)
            gContext->memory_allocated_maximum = gContext->memory_allocated;

#ifdef MASH_WARDEN_MEMORY_DEBUG
        printf("[Sandboxed object #%d] posix_memalign(%lu) -> ", gContext->sandboxed_object, (unsigned long) size);
        printMemoryUsage();
#endif

        size_t* ptr;
        int ret = gFunctions.posix_memalign(&ptr, alignment, size + 2 * sizeof(size_t));
        ptr[0] = (size_t) ((char*) (ptr + 2) + size);
        ptr[1] = size;

        *memptr = &ptr[2];

        return ret;
    }
    else
    {
        return gFunctions.posix_memalign(memptr, alignment, size);
    }
}


void free(void* ptr)
{
    INIT_WARDEN();

    if (ptr == 0)
    	return;

    if (gUnsafeFreeEnabled == 1)
    {
        int ret = setjmp(gEnvironment);
        
        if (ret == 0)
        {
            gFunctions.signal(SIGSEGV, free_segv_handler);

            size_t* p = (size_t*) ptr - 2;
            size_t size = p[1];

            if ((size > 0) && (*p == (size_t) ((char*) (p + 2) + size)))
                ptr = (void*) p;
        }

        gFunctions.signal(SIGSEGV, SIG_DFL);

        gFunctions.free(ptr);
    }
    else
    {
        if (gContext && (gContext->memory_limit > 0))
        {
#ifdef MASH_WARDEN_MEMORY_DEBUG
            size_t before = gContext->memory_allocated;
#endif

            size_t* internal_ptr = ((size_t*) ptr) - 2;
            gContext->memory_allocated -= internal_ptr[1];

#ifdef MASH_WARDEN_MEMORY_DEBUG
            printf("[Sandboxed object #%d] free(%lu) -> ", gContext->sandboxed_object, (unsigned long) (before - gContext->memory_allocated));
            printMemoryUsage();
#endif

            gFunctions.free(internal_ptr);
        }
        else
        {
            gFunctions.free(ptr);
        }
    }
}


/************************** FORBIDDEN SYSTEM CALLS ****************************/

int system(const char *command)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "system");
        gContext = 0;
        exit(2);
    }

    return gFunctions.system(command);
}


FILE* popen(const char *command, const char *mode)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "popen");
        gContext = 0;
        exit(2);
    }

    return gFunctions.popen(command, mode);
}


int pclose(FILE *stream)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "pclose");
        gContext = 0;
        exit(2);
    }

    return gFunctions.pclose(stream);
}


FILE* fdopen(int fildes, const char *mode)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "fdopen");
        gContext = 0;
        exit(2);
    }

    return gFunctions.fdopen(fildes, mode);
}


FILE* fopen(const char * filename, const char * mode)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "fopen");
        gContext = 0;
        exit(2);
    }

    return gFunctions.fopen(filename, mode);
}


FILE* freopen(const char * filename, const char * mode, FILE * stream)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "freopen");
        gContext = 0;
        exit(2);
    }

    return gFunctions.freopen(filename, mode, stream);
}


int fclose(FILE *stream)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "fclose");
        gContext = 0;
        exit(2);
    }

    return gFunctions.fclose(stream);
}


FILE* tmpfile(void)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "tmpfile");
        gContext = 0;
        exit(2);
    }

    return gFunctions.tmpfile();
}


void* dlopen(const char* path, int mode)
{
    INIT_WARDEN();
        
    if (gContext && !(gContext->exceptions & WARDEN_EXCEPTION_DLOPEN))
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "dlopen");
        gContext = 0;
        exit(2);
    }

    return gFunctions.dlopen(path, mode);
}


int dlclose(void* handle)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "dlclose");
        gContext = 0;
        exit(2);
    }

    return gFunctions.dlclose(handle);
}


int socket(int domain, int type, int protocol)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "socket");
        gContext = 0;
        exit(2);
    }

    return gFunctions.socket(domain, type, protocol);
}


int close(int fildes)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "close");
        gContext = 0;
        exit(2);
    }

    return gFunctions.close(fildes);
}


int execve(const char *path, char *const argv[], char *const envp[])
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execve");
        gContext = 0;
        exit(2);
    }

    return gFunctions.execve(path, argv, envp);
}


int execl(const char *path, const char *arg0, ... /*, (char *)0 */)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execl");
        gContext = 0;
        exit(2);
    }

    char** args;
    unsigned int nb_args = 0;
	va_list argp;
	char* p;
    
    va_start(argp, arg0);

	while ((p = va_arg(argp, char*)) != NULL)
		++nb_args;

	va_end(argp);

    args = (char**) malloc((nb_args + 1) * sizeof(char*));
    memset(args, 0, (nb_args + 1) * sizeof(char*));

    va_start(argp, arg0);

    unsigned int i = 0;
    for (i = 0; i < nb_args; ++i)
	{
        p = (char*) va_arg(argp, char*);
        args[i] = (char*) malloc(strlen(p) + 1);
        memcpy(args[i], p, strlen(p) + 1);
	}
	
	va_end(argp);

    int result = gFunctions.execv(path, args);

    for (i = 0; i < nb_args; ++i)
        free(args[i]);
    free(args);
    
    return result;
}


int execle(const char *path, const char *arg0, ... /*, (char *)0, char *const envp[] */)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execle");
        gContext = 0;
        exit(2);
    }

    char** args;
    unsigned int nb_args = 0;
	va_list argp;
	char* p;
    
    va_start(argp, arg0);

	while ((p = va_arg(argp, char*)) != NULL)
		++nb_args;

	va_end(argp);

    args = (char**) malloc((nb_args + 1) * sizeof(char*));
    memset(args, 0, (nb_args + 1) * sizeof(char*));

    va_start(argp, arg0);

    unsigned int i = 0;
    for (i = 0; i < nb_args; ++i)
	{
        p = va_arg(argp, char*);
        args[i] = (char*) malloc(strlen(p) + 1);
        memcpy(args[i], p, strlen(p) + 1);
	}

    p = va_arg(argp, char*);    // The 0
    
    char** environ = va_arg(argp, char**);
	
	va_end(argp);

    int result = gFunctions.execve(path, args, environ);

    for (i = 0; i < nb_args; ++i)
        free(args[i]);
    free(args);
    
    return result;
}


int execlp(const char *file, const char *arg0, ... /*, (char *)0 */)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execlp");
        gContext = 0;
        exit(2);
    }

    char** args;
    unsigned int nb_args = 0;
	va_list argp;
	char* p;
    
    va_start(argp, arg0);

	while ((p = va_arg(argp, char*)) != NULL)
		++nb_args;

	va_end(argp);

    args = (char**) malloc((nb_args + 1) * sizeof(char*));
    memset(args, 0, (nb_args + 1) * sizeof(char*));

    va_start(argp, arg0);

    unsigned int i = 0;
    for (i = 0; i < nb_args; ++i)
	{
        p = va_arg(argp, char*);
        args[i] = (char*) malloc(strlen(p) + 1);
        memcpy(args[i], p, strlen(p) + 1);
	}
	
	va_end(argp);

    int result = gFunctions.execvp(file, args);

    for (i = 0; i < nb_args; ++i)
        free(args[i]);
    free(args);
    
    return result;
}


int execv(const char *path, char *const argv[])
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execv");
        gContext = 0;
        exit(2);
    }

    return gFunctions.execv(path, argv);
}


int execvp(const char *file, char *const argv[])
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execvp");
        gContext = 0;
        exit(2);
    }

    return gFunctions.execvp(file, argv);
}


int execvP(const char *file, const char *search_path, char *const argv[])
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "execvP");
        gContext = 0;
        exit(2);
    }

    return gFunctions.execvP(file, search_path, argv);
}


void exit(int status)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "exit");
        gContext = 0;
        exit(2);
    }

    gFunctions.exit(status);
}


void _Exit(int status)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "_Exit");
        gContext = 0;
        exit(2);
    }

    gFunctions._Exit(status);
}


void _exit(int status)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "_exit");
        gContext = 0;
        exit(2);
    }

    gFunctions._exit(status);
}


int atexit(void (*func)(void))
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "atexit");
        gContext = 0;
        exit(2);
    }

    gFunctions.atexit(func);
}


pid_t wait(int *stat_loc)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "wait");
        gContext = 0;
        exit(2);
    }

    return gFunctions.wait(stat_loc);
}


pid_t wait3(int *stat_loc, int options, struct rusage *rusage)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "wait3");
        gContext = 0;
        exit(2);
    }

    return gFunctions.wait3(stat_loc, options, rusage);
}


pid_t wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "wait4");
        gContext = 0;
        exit(2);
    }

    return gFunctions.wait4(pid, stat_loc, options, rusage);
}


pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "waitpid");
        gContext = 0;
        exit(2);
    }

    return gFunctions.waitpid(pid, stat_loc, options);
}


unsigned int sleep(unsigned int seconds)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sleep");
        gContext = 0;
        exit(2);
    }

    return gFunctions.sleep(seconds);
}


int usleep(useconds_t useconds)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "usleep");
        gContext = 0;
        exit(2);
    }

    return gFunctions.usleep(useconds);
}


int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "nanosleep");
        gContext = 0;
        exit(2);
    }

    return gFunctions.nanosleep(rqtp, rmtp);
}


pid_t fork(void)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "fork");
        gContext = 0;
        exit(2);
    }

    return gFunctions.fork();
}


pid_t vfork(void)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "vfork");
        gContext = 0;
        exit(2);
    }

    return gFunctions.vfork();
}


sig_t signal(int sig, sig_t func)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "signal");
        gContext = 0;
        exit(2);
    }

    return gFunctions.signal(sig, func);
}


int sigaction(int sig, const struct sigaction * act, struct sigaction * oact)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sigaction");
        gContext = 0;
        exit(2);
    }

    return gFunctions.sigaction(sig, act, oact);
}


int sigaltstack(const stack_t * ss, stack_t * oss)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sigaltstack");
        gContext = 0;
        exit(2);
    }

    return gFunctions.sigaltstack(ss, oss);
}


int sigprocmask(int how, const sigset_t * set, sigset_t * oset)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sigprocmask");
        gContext = 0;
        exit(2);
    }

    return gFunctions.sigprocmask(how, set, oset);
}


int sigsuspend(const sigset_t *sigmask)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sigsuspend");
        gContext = 0;
        exit(2);
    }

    return gFunctions.sigsuspend(sigmask);
}


int siginterrupt(int sig, int flag)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "siginterrupt");
        gContext = 0;
        exit(2);
    }

    return gFunctions.siginterrupt(sig, flag);
}


int kill(pid_t pid, int sig)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "kill");
        gContext = 0;
        exit(2);
    }

    return gFunctions.kill(pid, sig);
}


void longjmp(jmp_buf env, int val)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "longjmp");
        gContext = 0;
        exit(2);
    }

    gFunctions.longjmp(env, val);
}


void longjmperror(void)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "longjmperror");
        gContext = 0;
        exit(2);
    }

    gFunctions.longjmperror();
}


int _setjmp(jmp_buf env)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "setjmp");
        gContext = 0;
        exit(2);
    }

    return gFunctions._setjmp(env);
}


void siglongjmp(sigjmp_buf env, int val)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "siglongjmp");
        gContext = 0;
        exit(2);
    }

    gFunctions.siglongjmp(env, val);
}


int __sigsetjmp(sigjmp_buf env, int savemask)
{
    INIT_WARDEN();
        
    if (gContext)
    {
        (*gListener)(gContext, WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, "sigsetjmp");
        gContext = 0;
        exit(2);
    }

    return gFunctions.__sigsetjmp(env, savemask);
}

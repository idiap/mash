#ifndef _MASH_SANDBOX_WARDEN_H_
#define _MASH_SANDBOX_WARDEN_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _tWardenStatus
{
    WARDEN_STATUS_NONE,
    WARDEN_STATUS_FAILURE,
    WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT,
    WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL,
} tWardenStatus;


typedef enum _tWardenExceptions
{
    WARDEN_EXCEPTION_DLOPEN = 1,
} tWardenExceptions;


typedef struct _tWardenContext
{
    unsigned int    sandboxed_object;          /* ID of the sandboxed object */
    size_t          memory_allocated;          /* Memory currently allocated */
    size_t          memory_allocated_maximum;  /* Maximum memory allocated */
    size_t          memory_limit;              /* Maximum memory that can be allocated */
    unsigned int    exceptions;                /* The authorized system calls */
} tWardenContext;


typedef void (*tWardenListener)(tWardenContext* pContext, tWardenStatus status,
                                const char* details);

void setWardenListener(tWardenListener listener);
void setWardenContext(tWardenContext* pContext);
tWardenContext* getWardenContext();

void wardenEnableUnsafeFree();
void wardenDisableUnsafeFree();

#ifdef __cplusplus
}
#endif

#endif

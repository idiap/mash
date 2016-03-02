#include <sandbox/warden.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "sandbox_tests.h"

using namespace std;


void listener(tWardenContext* pContext, tWardenStatus status, const char* details)
{
    CHECK(pContext);
    CHECK_EQUAL(WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, status);
    CHECK_EQUAL(string("execvp"), details);
    setWardenContext(0);
    _exit(0);
}


int main(int argc, char** argv)
{
    setWardenListener(&listener);

    tWardenContext context;
    context.sandboxed_object            = 0;
    context.memory_allocated            = 0;
    context.memory_allocated_maximum    = 0;
    context.memory_limit                = 0;
    
    setWardenContext(&context);

    char* myargs[] = 
    {
        "ls",
        0,
    };
    
    execvp("ls", myargs);
    
    CHECK(false);

    return 0;
}

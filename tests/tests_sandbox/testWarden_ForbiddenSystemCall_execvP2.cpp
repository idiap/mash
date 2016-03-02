#include <sandbox/warden.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "tests.h"

using namespace std;


void listener(tWardenContext* pContext, tWardenStatus status, const char* details)
{
    CHECK_EXIT(pContext);
    CHECK_EQUAL_EXIT(WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL, status);
    CHECK_EQUAL_EXIT(string("execvP"), details);
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
    
    execvP("ls", ".", myargs);
    
    CHECK(false);

    return 0;
}

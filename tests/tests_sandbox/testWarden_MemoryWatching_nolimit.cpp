#include <sandbox/warden.h>
#include <iostream>
#include "sandbox_tests.h"

using namespace std;


void listener(tWardenContext* pContext, tWardenStatus status, const char* details)
{
    CHECK(false);
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

    unsigned char* p = (unsigned char*) malloc(100);

    CHECK(p);
    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(0, context.memory_allocated_maximum);

    free(p);
    p = 0;

    setWardenContext(0);

    return 0;
}

#include <sandbox/warden.h>
#include <iostream>
#include "sandbox_tests.h"

using namespace std;


void listener(tWardenContext* pContext, tWardenStatus status, const char* details)
{
    CHECK(pContext);
    CHECK_EQUAL(WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT, status);
    CHECK_EQUAL(0, details);
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
    context.memory_limit                = 1024;    
    
    setWardenContext(&context);

    unsigned char* p = (unsigned char*) realloc(0, 100);

    CHECK(p);
    CHECK_EQUAL(100, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    p = (unsigned char*) realloc(p, 50);

    CHECK_EQUAL(50, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    p = (unsigned char*) realloc(p, 200);

    CHECK_EQUAL(200, context.memory_allocated);
    CHECK_EQUAL(200, context.memory_allocated_maximum);

    p = (unsigned char*) realloc(p, 2048);

    CHECK(false);

    return 0;
}

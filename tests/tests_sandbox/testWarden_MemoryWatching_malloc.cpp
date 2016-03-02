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

    unsigned char* p = (unsigned char*) malloc(100);

    CHECK(p);
    CHECK_EQUAL(100, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    free(p);
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    p = (unsigned char*) malloc(50);

    CHECK(p);
    CHECK_EQUAL(50, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    free(p);
    p = 0;


    setWardenContext(0);

    p = (unsigned char*) malloc(100);

    CHECK(p);
    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    free(p);
    p = 0;

    setWardenContext(&context);


    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(100, context.memory_allocated_maximum);

    p = (unsigned char*) malloc(200);

    CHECK(p);
    CHECK_EQUAL(200, context.memory_allocated);
    CHECK_EQUAL(200, context.memory_allocated_maximum);

    free(p);
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(200, context.memory_allocated_maximum);

    p = (unsigned char*) malloc(2048);

    CHECK(false);

    return 0;
}

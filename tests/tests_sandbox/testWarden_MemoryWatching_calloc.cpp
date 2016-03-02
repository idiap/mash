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
    struct tObject
    {
        unsigned int a;
        unsigned int b;
        unsigned int c;
        unsigned int d;
    };


    setWardenListener(&listener);

    tWardenContext context;
    context.sandboxed_object            = 0;
    context.memory_allocated            = 0;
    context.memory_allocated_maximum    = 0;
    context.memory_limit                = 25 * sizeof(tObject);    

    setWardenContext(&context);

    tObject* p = (tObject*) calloc(10, sizeof(tObject));

    CHECK(p);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    free(p);
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    p = (tObject*) calloc(5, sizeof(tObject));

    CHECK(p);
    CHECK_EQUAL(5 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    free(p);
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);


    setWardenContext(0);

    p = (tObject*) calloc(10, sizeof(tObject));

    CHECK(p);
    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    free(p);
    p = 0;

    setWardenContext(&context);


    p = (tObject*) calloc(20, sizeof(tObject));

    CHECK(p);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated_maximum);

    free(p);
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated_maximum);

    p = (tObject*) calloc(30, sizeof(tObject));

    CHECK(false);

    return 0;
}

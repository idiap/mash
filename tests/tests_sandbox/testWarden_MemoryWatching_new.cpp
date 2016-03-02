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

    tObject* p = new tObject[10];

    CHECK(p);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    delete[] p;
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    p = new tObject[5];

    CHECK(p);
    CHECK_EQUAL(5 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    delete[] p;
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);


    setWardenContext(0);

    p = new tObject[10];

    CHECK(p);
    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(10 * sizeof(tObject), context.memory_allocated_maximum);

    delete[] p;
    p = 0;

    setWardenContext(&context);


    p = new tObject[20];
    
    CHECK(p);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated_maximum);

    delete[] p;
    p = 0;

    CHECK_EQUAL(0, context.memory_allocated);
    CHECK_EQUAL(20 * sizeof(tObject), context.memory_allocated_maximum);

    p = new tObject[30];

    CHECK(false);

    return 0;
}

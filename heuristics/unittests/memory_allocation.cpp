#include <mash/heuristic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace Mash;


class MemoryAllocationHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    MemoryAllocationHeuristic() {}
    virtual ~MemoryAllocationHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        char* ptr = (char*) malloc(256 * 1024 * 1024);

        // Crash to inform the unit test about the failure
        int* p = 0;
        *p = 4;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new MemoryAllocationHeuristic();
}

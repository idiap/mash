#include <mash/heuristic.h>

using namespace Mash;


int crash()
{
    int* pointer = 0;
    *pointer = 4;
    
    return 0;
}


static int A = crash();


class CrashHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    CrashHeuristic()
    {
    }

    virtual ~CrashHeuristic()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new CrashHeuristic();
}

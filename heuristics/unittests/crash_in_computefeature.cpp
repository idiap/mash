#include <mash/heuristic.h>

using namespace Mash;


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
        int* pointer = 0;
        *pointer = 4;

        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new CrashHeuristic();
}

#include <mash/heuristic.h>

using namespace Mash;


class MemoryHogHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    MemoryHogHeuristic() {}
    virtual ~MemoryHogHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        char* buffer = new char[512 * 1024 * 1024];
        delete[] buffer;
        
        return (buffer != 0 ? 1 : 0);
    }

    virtual void prepareForImage()
    {
    }
    
    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new MemoryHogHeuristic();
}

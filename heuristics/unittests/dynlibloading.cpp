#include <mash/heuristic.h>
#include <dlfcn.h>
#include <stdio.h>

using namespace Mash;


class DynlibLoadingHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    DynlibLoadingHeuristic() {}
    virtual ~DynlibLoadingHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        void* handle = dlopen("heuristics/unittests/libfork.so", RTLD_NOW | RTLD_LOCAL);
        if (handle == 0)
            handle = dlopen("../heuristics/unittests/libfork.so", RTLD_NOW | RTLD_LOCAL);
        
        if (handle != 0)
        {
            printf("PWND!\n");
            dlclose(handle);

            // Crash so the unit test can notice the failure
            int* pointer = 0;
            *pointer = 4;
        }
    }
    
    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new DynlibLoadingHeuristic();
}

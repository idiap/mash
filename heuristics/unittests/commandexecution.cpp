#include <mash/heuristic.h>
#include <stdlib.h>

using namespace Mash;


class CommandExecutionHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    CommandExecutionHeuristic() {}
    virtual ~CommandExecutionHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        if (system("echo 'PWND by system()'") == 0)
        {
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
    return new CommandExecutionHeuristic();
}

#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


class TestInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    TestInstrument()
    {
    }

    virtual ~TestInstrument()
    {
    }
    

    //_____ Classifier-related events __________
public:
    virtual void onPlannerLearningStarted(ITask* task)
    {
        unsigned int nbActions = task->nbActions();
        
        scalar_t reward;
        for (unsigned int i = 0; i < nbActions; ++i)
            task->performAction(i, &reward);
        
        // Crash if we succeed at resetting the task
        task->reset();
        if (task->reset())
        {
            int* p = 0; 
            *p = 4;
        }
    }
};


extern "C" Instrument* new_instrument()
{
    return new TestInstrument();
}

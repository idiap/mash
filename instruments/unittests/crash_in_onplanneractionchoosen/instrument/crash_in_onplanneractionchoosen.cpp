#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


class CrashInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    CrashInstrument()
    {
    }

    virtual ~CrashInstrument()
    {
    }
    

    //_____ Goal-planner-related events __________
public:
    virtual void onPlannerActionChoosen(ITask* task, unsigned int action,
                                        scalar_t reward, tResult result)
    {
        int* p = 0;
        *p = 4;
    }
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

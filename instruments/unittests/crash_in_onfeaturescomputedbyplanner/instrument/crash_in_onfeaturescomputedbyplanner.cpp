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
    virtual void onFeaturesComputedByPlanner(unsigned int sequence,
                                             unsigned int view,
                                             unsigned int image,
                                             const coordinates_t& coords,
                                             unsigned int roiExtent,
                                             unsigned int heuristic,
                                             unsigned int nbFeatures,
                                             unsigned int* indexes,
                                             scalar_t* values)
    {
        int* p = 0;
        *p = 4;
    }
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

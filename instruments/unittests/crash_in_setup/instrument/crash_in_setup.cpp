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
    

    //_____ Classifier-related events __________
public:
    virtual void setup(const tExperimentParametersList& parameters)
    {
        int* p = 0;
        *p = 4;
    }
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

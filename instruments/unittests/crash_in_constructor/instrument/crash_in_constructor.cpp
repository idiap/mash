#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


class CrashInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    CrashInstrument()
    {
        int* p = 0;
        *p = 4;
    }

    virtual ~CrashInstrument()
    {
    }
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

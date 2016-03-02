#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;

int crash()
{
    int* pointer = 0;
    *pointer = 4;
    
    return 0;
}


static int A = crash();



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
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

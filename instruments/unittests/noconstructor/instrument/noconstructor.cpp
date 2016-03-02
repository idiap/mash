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
};

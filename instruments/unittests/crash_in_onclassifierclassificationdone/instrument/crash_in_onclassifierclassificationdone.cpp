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
    virtual void onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                unsigned int image,
                                                unsigned int original_image,
                                                const coordinates_t& position,
                                                const Classifier::tClassificationResults& results,
                                                tClassificationError error)
    {
        int* p = 0;
        *p = 4;
    }
};


extern "C" Instrument* new_instrument()
{
    return new CrashInstrument();
}

#include <UnitTest++.h>
#include <mash-instrumentation/instruments_manager.h>

using namespace Mash;


SUITE(InstrumentsManagerSuite)
{
    TEST(LoadingOfInstrumentByName)
    {
        InstrumentsManager manager("instruments");
        
        Instrument* pInstrument = manager.create("unittests/nooperation");
        CHECK(pInstrument);
        
        delete pInstrument;
    }
    

    TEST(LoadingOfUnknownInstrumentFail)
    {
        InstrumentsManager manager("instruments");
        
        Instrument* pInstrument = manager.create("unknown");
        CHECK(!pInstrument);
        CHECK_EQUAL(ERROR_INSTRUMENT_LOADING, manager.getLastError());
    }


    TEST(LoadingOfInstrumentLackingAConstructorFail)
    {
        InstrumentsManager manager("instruments");
        
        Instrument* pInstrument = manager.create("unittests/noconstructor");
        CHECK(!pInstrument);
        CHECK_EQUAL(ERROR_INSTRUMENT_CONSTRUCTOR, manager.getLastError());
    }
}

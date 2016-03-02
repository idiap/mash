#include <mash-instrumentation/sandboxed_instruments_set.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    SandboxedInstrumentsSet sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "instruments/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setInstrumentsFolder("instruments"));

    CHECK_EQUAL(0, sandbox.loadInstrumentPlugin("unittests/crash_in_onclassifierclassificationdone"));

    CHECK(sandbox.createInstruments());

    MockInputSet inputSet;

    CHECK(sandbox.onClassifierTestStarted(&inputSet));

    coordinates_t position;
    position.x = 63;
    position.y = 63;

    Classifier::tClassificationResults results;

    CHECK(!sandbox.onClassifierClassificationDone(&inputSet, 1, 10, position, results,
                                                  Instrument::CLASSIFICATION_ERROR_WRONG_CLASSIFICATION));

    CHECK_EQUAL(ERROR_INSTRUMENT_CRASHED, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

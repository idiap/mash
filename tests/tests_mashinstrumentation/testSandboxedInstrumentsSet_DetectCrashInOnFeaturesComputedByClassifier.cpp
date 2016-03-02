#include <mash-instrumentation/sandboxed_instruments_set.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockTask.h"

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

    CHECK_EQUAL(0, sandbox.loadInstrumentPlugin("unittests/crash_in_onfeaturescomputedbyclassifier"));

    CHECK(sandbox.createInstruments());

    coordinates_t position;
    position.x = 63;
    position.y = 63;
    
    unsigned int index = 0;
    scalar_t feature;

    CHECK(!sandbox.onFeaturesComputedByClassifier(false, true, 0, 0, position, 63, 0, 1, &index, &feature));
    CHECK_EQUAL(ERROR_INSTRUMENT_CRASHED, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

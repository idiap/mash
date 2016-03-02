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

    CHECK_EQUAL(0, sandbox.loadInstrumentPlugin("unittests/input_set_tester"));

    CHECK(sandbox.createInstruments());

    MockInputSet inputSet;

    CHECK(sandbox.onClassifierTrainingStarted(&inputSet));

    // Note: the onClassifierTrainingStarted() method of the sandbox calls those
    // methods one time each before calling the onClassifierTrainingStarted()
    // method of the instruments
    CHECK_EQUAL(2, inputSet.calls_counter_nbHeuristics);
    CHECK_EQUAL(2, inputSet.calls_counter_nbImages);
    CHECK_EQUAL(2, inputSet.calls_counter_nbLabels);
    CHECK_EQUAL(2, inputSet.calls_counter_roiExtent);

    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_nbFeatures);
    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_heuristicName);

    // Should not be able to compute features
    CHECK_EQUAL(0, inputSet.calls_counter_computeSomeFeatures);
    CHECK_EQUAL(0, inputSet.nbComputedFeatures);

    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_objectsInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_negativesInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_imageSize);
    
    return 0;
}

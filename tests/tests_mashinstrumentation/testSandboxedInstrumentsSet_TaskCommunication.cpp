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

    CHECK_EQUAL(0, sandbox.loadInstrumentPlugin("unittests/task_tester"));

    CHECK(sandbox.createInstruments());

    MockTask task;

    CHECK(sandbox.onPlannerLearningStarted(&task));

    // Note: the onPlannerLearningStarted() method of the sandbox calls this
    // method one time each before calling the onPlannerLearningStarted() method
    // of the instruments
    CHECK_EQUAL(2, task.calls_counter_nbActions);
    
    CHECK_EQUAL(0, task.calls_counter_reset);
    CHECK_EQUAL(0, task.calls_counter_performAction);
    
    return 0;
}

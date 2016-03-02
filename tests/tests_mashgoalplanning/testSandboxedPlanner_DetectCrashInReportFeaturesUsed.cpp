#include <mash-goalplanning/sandboxed_planner.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockTask.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    SandboxedPlanner sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "goalplanners/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setPlannersFolder("goalplanners"));

    CHECK(sandbox.loadPlannerPlugin("unittests/crash_in_reportfeaturesused"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockTask task;

    CHECK(sandbox.learn(&task));

    unsigned int action;
    CHECK(sandbox.chooseAction(task.perception(), &action));
    
    tFeatureList features;

    CHECK(!sandbox.reportFeaturesUsed(task.perception(), features));
    CHECK_EQUAL(ERROR_PLANNER_CRASHED, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

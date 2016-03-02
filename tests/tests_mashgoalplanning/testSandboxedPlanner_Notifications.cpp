#include <mash-goalplanning/sandboxed_planner.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockTask.h"
#include "MockNotifier.h"

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

    CHECK(sandbox.loadPlannerPlugin("unittests/notifier_tester"));

    MockNotifier notifier;
    sandbox.setNotifier(&notifier);

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockTask task;

    CHECK(sandbox.learn(&task));

    CHECK_EQUAL(5, notifier.step);
    CHECK_EQUAL(10, notifier.nbTotalSteps);
    
    return 0;
}

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

    CHECK(sandbox.loadPlannerPlugin("unittests/task_tester"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockTask task;

    CHECK(sandbox.learn(&task));

    // Note: the learn() method of the sandbox calls this method one time
    // before calling the learn() method of the goal-planner
    CHECK_EQUAL(2, task.calls_counter_nbActions);
    
    CHECK(task.calls_counter_mode > 0);
    CHECK_EQUAL(1, task.calls_counter_nbTrajectories);
    CHECK_EQUAL(task.nbTrajectories(), task.calls_counter_trajectoryLength);
    CHECK_EQUAL(1, task.calls_counter_reset);
    CHECK_EQUAL(task.nbActions(), task.calls_counter_performAction);
    CHECK_EQUAL(0, task.calls_counter_suggestedAction);
        
    return 0;
}

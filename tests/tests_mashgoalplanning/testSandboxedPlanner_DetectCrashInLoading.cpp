#include <mash-goalplanning/sandboxed_planner.h>
#include <iostream>
#include <string>
#include "tests.h"

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

    CHECK(!sandbox.loadPlannerPlugin("unittests/crash_in_loading"));
    CHECK_EQUAL(ERROR_PLANNER_CRASHED, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

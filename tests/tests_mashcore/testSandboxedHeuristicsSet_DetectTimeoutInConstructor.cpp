#include <mash/sandboxed_heuristics_set.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    SandboxedHeuristicsSet sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "heuristics/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setHeuristicsFolder("heuristics"));

    CHECK_EQUAL(0, sandbox.loadHeuristicPlugin("unittests/timeout_in_constructor"));
    
    CHECK(!sandbox.createHeuristics());
    CHECK_EQUAL(ERROR_HEURISTIC_TIMEOUT, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    CHECK(!sandbox.sandboxController()->ping());

    return 0;
}

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

    // This heuristic will crash while loading if it succeeds at opening a file
    CHECK_EQUAL(-1, sandbox.loadHeuristicPlugin("unittests/fileopening_while_loading"));
    CHECK_EQUAL(ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL, sandbox.getLastError());
    
    return 0;
}

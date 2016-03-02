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

    CHECK_EQUAL(0, sandbox.loadHeuristicPlugin("unittests/timeout_in_finishforimage"));
    
    CHECK(sandbox.createHeuristics());
    
    CHECK(sandbox.init(0, 1, 63));

    CHECK(sandbox.prepareForSequence(0));

    Image image(49, 49);
    image.addPixelFormats(Image::PIXELFORMAT_ALL);

    CHECK(sandbox.prepareForImage(0, 0, 0, &image));

    CHECK(!sandbox.finishForImage(0));
    CHECK_EQUAL(ERROR_HEURISTIC_TIMEOUT, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    CHECK(!sandbox.sandboxController()->ping());
    
    return 0;
}

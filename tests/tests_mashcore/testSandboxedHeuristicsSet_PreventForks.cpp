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

    // This heuristic will crash in prepareForImage() if it succeeds at forking
    CHECK_EQUAL(0, sandbox.loadHeuristicPlugin("unittests/fork"));
    
    CHECK(sandbox.createHeuristics());
    
    CHECK(sandbox.init(0, 1, 63));

    CHECK(sandbox.prepareForSequence(0));

    Image image(127, 127);
    image.addPixelFormats(Image::PIXELFORMAT_ALL);

#if MASH_PLATFORM == MASH_PLATFORM_APPLE
    CHECK(sandbox.prepareForImage(0, 0, 0, &image));
    CHECK_EQUAL(ERROR_NONE, sandbox.getLastError());
#else
    CHECK(!sandbox.prepareForImage(0, 0, 0, &image));
    CHECK_EQUAL(ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL, sandbox.getLastError());
#endif
    
    return 0;
}

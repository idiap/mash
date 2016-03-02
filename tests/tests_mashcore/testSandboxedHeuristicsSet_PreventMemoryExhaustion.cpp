#include <mash/sandboxed_heuristics_set.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    OutStream::verbosityLevel = 3;
    
    SandboxedHeuristicsSet sandbox;
    tSandboxConfiguration configuration;
    
    configuration.verbosity = 3;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "heuristics/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setHeuristicsFolder("heuristics"));

    CHECK_EQUAL(0, sandbox.loadHeuristicPlugin("unittests/memory_allocation"));
    
    CHECK(sandbox.createHeuristics());
    
    CHECK(sandbox.init(0, 1, 63));

    CHECK(sandbox.prepareForSequence(0));

    Image image(127, 127);
    image.addPixelFormats(Image::PIXELFORMAT_ALL);

    CHECK(!sandbox.prepareForImage(0, 0, 0, &image));

    CHECK(!sandbox.prepareForImage(0, 0, 0, &image));
    CHECK_EQUAL(ERROR_SANDBOX_MEMORY_LIMIT_REACHED, sandbox.getLastError());
    
    return 0;
}

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

    CHECK_EQUAL(0, sandbox.loadHeuristicPlugin("unittests/nan"));
    
    CHECK(sandbox.createHeuristics());
    
    CHECK(sandbox.init(0, 1, 63));

    CHECK(sandbox.prepareForSequence(0));

    Image image(127, 127);
    image.addPixelFormats(Image::PIXELFORMAT_ALL);

    CHECK(sandbox.prepareForImage(0, 0, 0, &image));

    coordinates_t coords;
    coords.x = 63;
    coords.y = 63;
    
    CHECK(sandbox.prepareForCoordinates(0, coords));

    unsigned int feature = 0;
    scalar_t value;

    CHECK(!sandbox.computeSomeFeatures(0, 1, &feature, &value));
    CHECK_EQUAL(ERROR_FEATURE_IS_NAN, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

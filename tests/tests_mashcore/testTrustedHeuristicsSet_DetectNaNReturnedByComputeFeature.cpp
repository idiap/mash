#include <mash/trusted_heuristics_set.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedHeuristicsSet trusted;

    trusted.configure("logs");
    
    CHECK(trusted.setHeuristicsFolder("heuristics"));

    CHECK_EQUAL(0, trusted.loadHeuristicPlugin("unittests/nan"));
    
    CHECK(trusted.createHeuristics());
    
    CHECK(trusted.init(0, 1, 63));

    CHECK(trusted.prepareForSequence(0));

    Image image(127, 127);
    image.addPixelFormats(Image::PIXELFORMAT_ALL);

    CHECK(trusted.prepareForImage(0, 0, 0, &image));

    coordinates_t coords;
    coords.x = 63;
    coords.y = 63;
    
    CHECK(trusted.prepareForCoordinates(0, coords));

    unsigned int feature = 0;
    scalar_t value;

    CHECK(!trusted.computeSomeFeatures(0, 1, &feature, &value));
    CHECK_EQUAL(ERROR_FEATURE_IS_NAN, trusted.getLastError());
    
    return 0;
}

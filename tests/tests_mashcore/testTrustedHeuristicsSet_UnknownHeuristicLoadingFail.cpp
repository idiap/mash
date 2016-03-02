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

    CHECK_EQUAL(-1, trusted.loadHeuristicPlugin("unknown"));
    CHECK_EQUAL(ERROR_DYNLIB_LOADING, trusted.getLastError());
    
    return 0;
}

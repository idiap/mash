#include <mash-instrumentation/trusted_instruments_set.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedInstrumentsSet trusted;

    trusted.configure("logs", "out");
    
    CHECK(trusted.setInstrumentsFolder("instruments"));

    CHECK_EQUAL(0, trusted.loadInstrumentPlugin("unittests/nooperation"));

    CHECK(trusted.createInstruments());
    
    return 0;
}

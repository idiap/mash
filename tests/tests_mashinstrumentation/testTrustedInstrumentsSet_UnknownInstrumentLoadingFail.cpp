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

    CHECK_EQUAL(-1, trusted.loadInstrumentPlugin("unknown"));
    CHECK_EQUAL(ERROR_DYNLIB_LOADING, trusted.getLastError());
    
    return 0;
}

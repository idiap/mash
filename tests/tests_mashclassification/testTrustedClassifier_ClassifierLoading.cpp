#include <mash-classification/trusted_classifier.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedClassifier trusted;

    trusted.configure("logs", "out");

    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/onefeature"));
    
    return 0;
}

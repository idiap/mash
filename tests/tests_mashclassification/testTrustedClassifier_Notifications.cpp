#include <mash-classification/trusted_classifier.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockInputSet.h"
#include "MockNotifier.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedClassifier trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/notifier_tester"));

    MockNotifier notifier;
    trusted.setNotifier(&notifier);

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(trusted.train(&inputSet, train_error));

    CHECK_EQUAL(5, notifier.step);
    CHECK_EQUAL(10, notifier.nbTotalSteps);
    
    return 0;
}

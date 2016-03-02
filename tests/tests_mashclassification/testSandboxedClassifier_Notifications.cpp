#include <mash-classification/sandboxed_classifier.h>
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
    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(sandbox.loadClassifierPlugin("unittests/notifier_tester"));

    MockNotifier notifier;
    sandbox.setNotifier(&notifier);

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(sandbox.train(&inputSet, train_error));

    CHECK_EQUAL(5, notifier.step);
    CHECK_EQUAL(10, notifier.nbTotalSteps);
    
    return 0;
}

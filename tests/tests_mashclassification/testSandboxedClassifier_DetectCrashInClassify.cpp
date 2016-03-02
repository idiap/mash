#include <mash-classification/sandboxed_classifier.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockInputSet.h"

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

    CHECK(sandbox.loadClassifierPlugin("unittests/crash_in_classify"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(sandbox.train(&inputSet, train_error));

    coordinates_t position;
    position.x = 63;
    position.y = 63;

    Classifier::tClassificationResults results;

    CHECK(!sandbox.classify(&inputSet, 0, position, results));
    CHECK_EQUAL(ERROR_CLASSIFIER_CRASHED, sandbox.getLastError());
    CHECK(!sandbox.getContext().empty());
    
    return 0;
}

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

    CHECK(sandbox.loadClassifierPlugin("unittests/input_set_tester"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(sandbox.train(&inputSet, train_error));

    CHECK_EQUAL(-HUGE_VAL, train_error);

    // Note: the train() method of the sandbox calls those methods one time each
    // before calling the train() method of the classifier
    CHECK_EQUAL(2, inputSet.calls_counter_nbHeuristics);
    CHECK_EQUAL(2, inputSet.calls_counter_nbImages);
    CHECK_EQUAL(2, inputSet.calls_counter_nbLabels);
    CHECK_EQUAL(2, inputSet.calls_counter_roiExtent);

    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_nbFeatures);
    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_heuristicName);
    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_heuristicSeed);
    CHECK_EQUAL(inputSet.nbImages() * inputSet.nbHeuristics(), inputSet.calls_counter_computeSomeFeatures);
    CHECK_EQUAL(inputSet.nbImages() * inputSet.nbFeaturesTotal(), inputSet.nbComputedFeatures);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_objectsInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_negativesInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_imageSize);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_imageInTestSet);
    
    return 0;
}

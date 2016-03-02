#include <mash-classification/trusted_classifier.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedClassifier trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/input_set_tester"));

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(trusted.train(&inputSet, train_error));

    CHECK_EQUAL(1, inputSet.calls_counter_nbHeuristics);
    CHECK_EQUAL(1, inputSet.calls_counter_nbImages);
    CHECK_EQUAL(1, inputSet.calls_counter_nbLabels);
    CHECK_EQUAL(1, inputSet.calls_counter_roiExtent);

    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_nbFeatures);
    CHECK_EQUAL(inputSet.nbHeuristics(), inputSet.calls_counter_heuristicName);
    CHECK_EQUAL(inputSet.nbImages() * inputSet.nbHeuristics(), inputSet.calls_counter_computeSomeFeatures);
    CHECK_EQUAL(inputSet.nbImages() * inputSet.nbFeaturesTotal(), inputSet.nbComputedFeatures);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_objectsInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_negativesInImage);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_imageSize);
    CHECK_EQUAL(inputSet.nbImages(), inputSet.calls_counter_imageInTestSet);
    
    return 0;
}

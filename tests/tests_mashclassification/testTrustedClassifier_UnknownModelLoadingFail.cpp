#include <mash-classification/trusted_classifier.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "tmp.model";


int main(int argc, char** argv)
{
    TrustedClassifier trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/model_loader", "unknown.model"));

    MockInputSet inputSet;

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) inputSet.nbHeuristics());

    CHECK(trusted.setup(parameters));

    CHECK(!trusted.loadModel(&inputSet));
    CHECK_EQUAL(ERROR_CLASSIFIER_MODEL_LOADING_FAILED, trusted.getLastError());
    
    return 0;
}

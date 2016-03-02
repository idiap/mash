#include <mash-classification/sandboxed_classifier.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "tmp.model";


int main(int argc, char** argv)
{
    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(sandbox.loadClassifierPlugin("unittests/model_loader", "unknown.model"));

    MockInputSet inputSet;

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) inputSet.nbHeuristics());

    CHECK(sandbox.setup(parameters));

    CHECK(!sandbox.loadModel(&inputSet));
    CHECK_EQUAL(ERROR_CLASSIFIER_MODEL_LOADING_FAILED, sandbox.getLastError());
    
    return 0;
}

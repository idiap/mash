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
    MockInputSet inputSet;

    // First create a fake model
    PredictorModel model;
    model.create(MODELFILE);
    for (unsigned int i = 0; i < inputSet.nbHeuristics(); ++i)
        model.addHeuristic(i, inputSet.heuristicName(i), 1000 * i);

    model.addHeuristic(inputSet.nbHeuristics(), "missing", 100);

    model.lockHeuristics();

    model.writer() << "OK" << endl;


    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(sandbox.loadClassifierPlugin("unittests/model_loader", MODELFILE));

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) inputSet.nbHeuristics());

    CHECK(sandbox.setup(parameters));

    CHECK(!sandbox.loadModel(&inputSet));

    CHECK_EQUAL(ERROR_CLASSIFIER_MODEL_MISSING_HEURISTIC, sandbox.getLastError());

    model.deleteFile();
    
    return 0;
}

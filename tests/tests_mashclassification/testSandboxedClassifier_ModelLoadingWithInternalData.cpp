#include <mash-classification/sandboxed_classifier.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "tmp.model";
const char* INTERNALFILE = "tmp.internal";


int main(int argc, char** argv)
{
    MockInputSet inputSet;

    // First create a fake model and internal data
    PredictorModel model;
    model.create(MODELFILE);
    for (unsigned int i = 0; i < inputSet.nbHeuristics(); ++i)
        model.addHeuristic(i, inputSet.heuristicName(i), 1000 * i);

    model.lockHeuristics();

    model.writer() << "OK" << endl;

    DataWriter internal;
    internal.open(INTERNALFILE);
    internal << "OK" << endl;


    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(sandbox.loadClassifierPlugin("unittests/model_loader", MODELFILE, INTERNALFILE));

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) inputSet.nbHeuristics());
    parameters["USE_INTERNAL_DATA"] = ArgumentsList(1);

    CHECK(sandbox.setup(parameters));

    CHECK(sandbox.loadModel(&inputSet));

    model.deleteFile();
    internal.deleteFile();
    
    return 0;
}

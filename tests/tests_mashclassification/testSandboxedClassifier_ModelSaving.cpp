#include <mash-classification/sandboxed_classifier.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "out/predictor.model";


int main(int argc, char** argv)
{
    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(sandbox.loadClassifierPlugin("unittests/model_saver"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(sandbox.train(&inputSet, train_error));

    tFeatureList features;

    CHECK(sandbox.reportFeaturesUsed(&inputSet, features));

    coordinates_t position;
    position.x = 63;
    position.y = 63;

    Classifier::tClassificationResults results;

    CHECK(sandbox.classify(&inputSet, 0, position, results));

    CHECK(sandbox.saveModel());


    PredictorModel model;
    const int64_t BUFFER_SIZE = 50;
    char buffer[BUFFER_SIZE];

    CHECK(model.open(MODELFILE));
    CHECK(model.isReadable());
    
    for (unsigned int i = 0; i < inputSet.nbHeuristics(); ++i)
        model.addHeuristic(i, inputSet.heuristicName(i));

    CHECK(model.lockHeuristics());

    CHECK_EQUAL(model.nbHeuristics(), inputSet.nbHeuristics());

    CHECK_EQUAL(0, model.reader().tell());
    CHECK(model.reader().readline(buffer, BUFFER_SIZE) > 0);
    CHECK_EQUAL( "OK", string(buffer));
    CHECK(model.reader().eof());

    model.deleteFile();
    
    return 0;
}

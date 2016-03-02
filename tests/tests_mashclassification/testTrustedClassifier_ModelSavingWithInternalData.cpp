#include <mash-classification/trusted_classifier.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockInputSet.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "out/predictor.model";
const char* INTERNALFILE = "out/predictor.internal";


int main(int argc, char** argv)
{
    TrustedClassifier trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/model_saver"));

    tExperimentParametersList parameters;
    parameters["USE_INTERNAL_DATA"] = ArgumentsList(1);

    CHECK(trusted.setup(parameters));

    MockInputSet inputSet;

    scalar_t train_error = -HUGE_VAL;
    CHECK(trusted.train(&inputSet, train_error));

    tFeatureList features;

    CHECK(trusted.reportFeaturesUsed(&inputSet, features));

    coordinates_t position;
    position.x = 63;
    position.y = 63;

    Classifier::tClassificationResults results;

    CHECK(trusted.classify(&inputSet, 0, position, results));

    CHECK(trusted.saveModel());


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
    
    DataReader internal;
    CHECK(internal.open(INTERNALFILE));

    CHECK_EQUAL(0, internal.tell());
    CHECK(internal.readline(buffer, BUFFER_SIZE) > 0);
    CHECK_EQUAL( "OK", string(buffer));
    CHECK(internal.eof());

    internal.deleteFile();
    
    return 0;
}

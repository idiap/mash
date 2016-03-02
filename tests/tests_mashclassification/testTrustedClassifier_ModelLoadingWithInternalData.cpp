#include <mash-classification/trusted_classifier.h>
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


    TrustedClassifier trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setClassifiersFolder("classifiers"));

    CHECK(trusted.loadClassifierPlugin("unittests/model_loader", MODELFILE, INTERNALFILE));

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) inputSet.nbHeuristics());
    parameters["USE_INTERNAL_DATA"] = ArgumentsList(1);

    CHECK(trusted.setup(parameters));

    CHECK(trusted.loadModel(&inputSet));

    model.deleteFile();
    internal.deleteFile();
    
    return 0;
}

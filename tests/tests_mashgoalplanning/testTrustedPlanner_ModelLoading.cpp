#include <mash-goalplanning/trusted_planner.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockPerception.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "tmp.model";


int main(int argc, char** argv)
{
    MockPerception perception;

    // First create a fake model
    PredictorModel model;
    model.create(MODELFILE);
    for (unsigned int i = 0; i < perception.nbHeuristics(); ++i)
        model.addHeuristic(i, perception.heuristicName(i), 1000 * i);

    model.lockHeuristics();

    model.writer() << "OK" << endl;


    TrustedPlanner trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(trusted.loadPlannerPlugin("unittests/model_loader", MODELFILE));

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) perception.nbHeuristics());

    CHECK(trusted.setup(parameters));

    CHECK(trusted.loadModel(&perception));

    model.deleteFile();
    
    return 0;
}

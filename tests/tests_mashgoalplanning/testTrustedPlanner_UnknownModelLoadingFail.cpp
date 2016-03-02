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
    TrustedPlanner trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(trusted.loadPlannerPlugin("unittests/model_loader", MODELFILE));

    MockPerception perception;

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) perception.nbHeuristics());

    CHECK(trusted.setup(parameters));

    CHECK(!trusted.loadModel(&perception));
    CHECK_EQUAL(ERROR_PLANNER_MODEL_LOADING_FAILED, trusted.getLastError());
    
    return 0;
}

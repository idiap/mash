#include <mash-goalplanning/sandboxed_planner.h>
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


    SandboxedPlanner sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "goalplanners/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setPlannersFolder("goalplanners"));

    CHECK(sandbox.loadPlannerPlugin("unittests/model_loader", MODELFILE));

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) perception.nbHeuristics());

    CHECK(sandbox.setup(parameters));

    CHECK(sandbox.loadModel(&perception));

    model.deleteFile();
    
    return 0;
}

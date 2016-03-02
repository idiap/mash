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
    SandboxedPlanner sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "goalplanners/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setPlannersFolder("goalplanners"));

    CHECK(sandbox.loadPlannerPlugin("unittests/model_loader", "unknown.model"));

    MockPerception perceptron;

    tExperimentParametersList parameters;
    parameters["NB_HEURISTICS"] = ArgumentsList((int) perceptron.nbHeuristics());

    CHECK(sandbox.setup(parameters));

    CHECK(!sandbox.loadModel(&perceptron));
    CHECK_EQUAL(ERROR_PLANNER_MODEL_LOADING_FAILED, sandbox.getLastError());
    
    return 0;
}

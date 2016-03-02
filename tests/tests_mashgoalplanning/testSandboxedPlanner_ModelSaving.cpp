#include <mash-goalplanning/sandboxed_planner.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockTask.h"

using namespace Mash;
using namespace std;


const char* MODELFILE = "out/predictor.model";


int main(int argc, char** argv)
{
    SandboxedPlanner sandbox;
    tSandboxConfiguration configuration;

    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "goalplanners/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;

    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setPlannersFolder("goalplanners"));

    CHECK(sandbox.loadPlannerPlugin("unittests/model_saver"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockTask task;

    CHECK(sandbox.learn(&task));

    tFeatureList features;

    CHECK(sandbox.reportFeaturesUsed(task.perception(), features));

    coordinates_t position;
    position.x = 63;
    position.y = 63;

    CHECK(sandbox.saveModel());


    PredictorModel model;
    const int64_t BUFFER_SIZE = 50;
    char buffer[BUFFER_SIZE];

    CHECK(model.open(MODELFILE));
    CHECK(model.isReadable());
    
    for (unsigned int i = 0; i < task.perception()->nbHeuristics(); ++i)
        model.addHeuristic(i, task.perception()->heuristicName(i));

    CHECK(model.lockHeuristics());

    CHECK_EQUAL(model.nbHeuristics(), task.perception()->nbHeuristics());

    CHECK_EQUAL(0, model.reader().tell());
    CHECK(model.reader().readline(buffer, BUFFER_SIZE) > 0);
    CHECK_EQUAL( "OK", string(buffer));
    CHECK(model.reader().eof());

    model.deleteFile();
    
    return 0;
}

#include <mash-goalplanning/trusted_planner.h>
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
    TrustedPlanner trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(trusted.loadPlannerPlugin("unittests/model_saver"));

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockTask task;

    CHECK(trusted.learn(&task));

    tFeatureList features;

    CHECK(trusted.reportFeaturesUsed(task.perception(), features));

    CHECK(trusted.saveModel());


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

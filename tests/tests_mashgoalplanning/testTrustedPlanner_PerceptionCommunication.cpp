#include <mash-goalplanning/trusted_planner.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockTask.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedPlanner trusted;

    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(trusted.loadPlannerPlugin("unittests/perception_tester"));

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockTask task;

    CHECK(trusted.learn(&task));

    unsigned int action;
    CHECK(trusted.chooseAction(task.perception(), &action));

    CHECK_EQUAL(1, task.mockPerception.calls_counter_nbHeuristics);
    CHECK_EQUAL(1, task.mockPerception.calls_counter_nbViews);
    CHECK_EQUAL(1, task.mockPerception.calls_counter_roiExtent);

    CHECK_EQUAL(task.mockPerception.nbViews(), task.mockPerception.calls_counter_viewSize);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_nbFeatures);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_heuristicName);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_heuristicSeed);
    CHECK_EQUAL(task.mockPerception.nbViews() * task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_computeSomeFeatures);
    CHECK_EQUAL(task.mockPerception.nbViews() * task.mockPerception.nbFeaturesTotal(), task.mockPerception.nbComputedFeatures);
        
    return 0;
}

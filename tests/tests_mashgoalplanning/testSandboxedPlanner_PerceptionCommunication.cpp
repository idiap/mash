#include <mash-goalplanning/sandboxed_planner.h>
#include <iostream>
#include <string>
#include "tests.h"
#include "MockTask.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    SandboxedPlanner sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "goalplanners/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setPlannersFolder("goalplanners"));

    CHECK(sandbox.loadPlannerPlugin("unittests/perception_tester"));

    tExperimentParametersList parameters;

    CHECK(sandbox.setup(parameters));

    MockTask task;

    CHECK(sandbox.learn(&task));

    unsigned int action;
    CHECK(sandbox.chooseAction(task.perception(), &action));

    // Note: the learn() and chooseAction() methods of the sandbox calls this methods one
    // time each before calling the corresponding methods of the goal-planner
    CHECK_EQUAL(3, task.mockPerception.calls_counter_nbHeuristics);
    CHECK_EQUAL(3, task.mockPerception.calls_counter_nbViews);
    
    CHECK_EQUAL(1, task.mockPerception.calls_counter_roiExtent);
    CHECK_EQUAL(task.mockPerception.nbViews(), task.mockPerception.calls_counter_viewSize);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_nbFeatures);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_heuristicName);
    CHECK_EQUAL(task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_heuristicSeed);
    CHECK_EQUAL(task.mockPerception.nbViews() * task.mockPerception.nbHeuristics(), task.mockPerception.calls_counter_computeSomeFeatures);
    CHECK_EQUAL(task.mockPerception.nbViews() * task.mockPerception.nbFeaturesTotal(), task.mockPerception.nbComputedFeatures);
        
    return 0;
}

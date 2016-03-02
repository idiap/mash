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

    CHECK(trusted.loadPlannerPlugin("unittests/task_tester"));

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockTask task;

    CHECK(trusted.learn(&task));

    CHECK_EQUAL(1, task.calls_counter_mode);
    CHECK_EQUAL(1, task.calls_counter_nbActions);
    CHECK_EQUAL(1, task.calls_counter_nbTrajectories);
    CHECK_EQUAL(task.nbTrajectories(), task.calls_counter_trajectoryLength);
    CHECK_EQUAL(1, task.calls_counter_reset);
    CHECK_EQUAL(task.nbActions(), task.calls_counter_performAction);
    CHECK_EQUAL(task.nbActions(), task.calls_counter_suggestedAction);
        
    return 0;
}

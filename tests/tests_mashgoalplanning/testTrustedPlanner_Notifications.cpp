#include <mash-goalplanning/trusted_planner.h>
#include <iostream>
#include <string>
#include <math.h>
#include "tests.h"
#include "MockTask.h"
#include "MockNotifier.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedPlanner trusted;
    
    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(trusted.loadPlannerPlugin("unittests/notifier_tester"));

    MockNotifier notifier;
    trusted.setNotifier(&notifier);

    tExperimentParametersList parameters;

    CHECK(trusted.setup(parameters));

    MockTask task;

    CHECK(trusted.learn(&task));

    CHECK_EQUAL(5, notifier.step);
    CHECK_EQUAL(10, notifier.nbTotalSteps);
    
    return 0;
}

#include <mash-goalplanning/trusted_planner.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    TrustedPlanner trusted;

    trusted.configure("logs", "out");
    
    CHECK(trusted.setPlannersFolder("goalplanners"));

    CHECK(!trusted.loadPlannerPlugin("unittests/noconstructor"));
    CHECK_EQUAL(ERROR_PLANNER_CONSTRUCTOR, trusted.getLastError());
    
    return 0;
}

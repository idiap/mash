#include <UnitTest++.h>
#include <mash-goalplanning/planners_manager.h>

using namespace Mash;


SUITE(PlannersManagerSuite)
{
    TEST(LoadingOfSimplePlannerByName)
    {
        PlannersManager manager("goalplanners");
        
        Planner* pPlanner = manager.create("examples/random");
        CHECK(pPlanner);
        
        delete pPlanner;
    }
    

    TEST(LoadingOfAdvancedPlannerByName)
    {
        PlannersManager manager("goalplanners");
        
        Planner* pPlanner = manager.create("examples/advanced");
        CHECK(pPlanner);
        
        delete pPlanner;
    }


    TEST(LoadingOfUnknownPlannerFail)
    {
        PlannersManager manager("goalplanners");
        
        Planner* pPlanner = manager.create("unknown");
        CHECK(!pPlanner);
        CHECK_EQUAL(ERROR_PLANNER_LOADING, manager.getLastError());
    }


    TEST(LoadingOfPlannerLackingAConstructorFail)
    {
        PlannersManager manager("goalplanners");
        
        Planner* pPlanner = manager.create("unittests/noconstructor");
        CHECK(!pPlanner);
        CHECK_EQUAL(ERROR_PLANNER_CONSTRUCTOR, manager.getLastError());
    }
}

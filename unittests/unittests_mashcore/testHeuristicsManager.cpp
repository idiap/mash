#include <UnitTest++.h>
#include <mash/heuristics_manager.h>

using namespace Mash;


SUITE(HeuristicsManagerSuite)
{
    TEST(LoadingOfHeuristicByName)
    {
        HeuristicsManager manager("heuristics");
        
        Heuristic* pHeuristic = manager.create("examples/identity");
        CHECK(pHeuristic);
        
        delete pHeuristic;
    }
    

    TEST(LoadingOfUnknownHeuristicFail)
    {
        HeuristicsManager manager("heuristics");
        
        Heuristic* pHeuristic = manager.create("unknown");
        CHECK(!pHeuristic);
        CHECK_EQUAL(ERROR_DYNLIB_LOADING, manager.getLastError());
    }


    TEST(LoadingOfHeuristicLackingAConstructorFail)
    {
        HeuristicsManager manager("heuristics");
        
        Heuristic* pHeuristic = manager.create("unittests/noconstructor");
        CHECK(!pHeuristic);
        CHECK_EQUAL(ERROR_HEURISTIC_CONSTRUCTOR, manager.getLastError());
    }
}

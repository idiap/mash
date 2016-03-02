#include <UnitTest++.h>
#include <mash-utils/arguments_list.h>

using namespace Mash;

SUITE(ArgumentsListSuite)
{
    TEST(ListIsEmptyAtCreation)
    {
        ArgumentsList args;
        CHECK_EQUAL(0, args.size());
    }


    TEST(OneStringArgumentAtCreation)
    {
        ArgumentsList args("SOME_ARGUMENT");
        CHECK_EQUAL("SOME_ARGUMENT", args.getString(0));
    }        
        

    TEST(OneIntArgumentAtCreation)
    {
        ArgumentsList args(10);
        CHECK_EQUAL(10, args.getInt(0));
    }        


    TEST(OneFloatArgumentAtCreation)
    {
        ArgumentsList args(20.5f);
        CHECK_CLOSE(20.5f, args.getFloat(0), 1e-6f);
    }        


    TEST(OneArgument)
    {
        ArgumentsList args;
        args.add(1);
        CHECK_EQUAL(1, args.size());
    }        


    TEST(TwoArguments)
    {
        ArgumentsList args;
        args.add(1);
        args.add(2);
        CHECK_EQUAL(2, args.size());
    }        


    TEST(OneStringArgument)
    {
        ArgumentsList args;
        args.add("SOME_ARGUMENT");
        CHECK_EQUAL("SOME_ARGUMENT", args.getString(0));
    }        
        

    TEST(OneIntArgument)
    {
        ArgumentsList args;
        args.add(10);
        CHECK_EQUAL(10, args.getInt(0));
    }        


    TEST(OneFloatArgument)
    {
        ArgumentsList args;
        args.add(20.5f);
        CHECK_CLOSE(20.5f, args.getFloat(0), 1e-6f);
    }        


    TEST(ListAppending)
    {
        ArgumentsList list1;
        list1.add(1);
        list1.add(2);

        ArgumentsList list2;
        list2.add(3);
        list2.add(4);

        list1.append(list2);

        CHECK_EQUAL(4, list1.size());
        CHECK_EQUAL(1, list1.getInt(0));
        CHECK_EQUAL(2, list1.getInt(1));
        CHECK_EQUAL(3, list1.getInt(2));
        CHECK_EQUAL(4, list1.getInt(3));
    }        
}

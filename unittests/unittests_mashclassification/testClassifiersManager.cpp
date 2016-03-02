#include <UnitTest++.h>
#include <mash-classification/classifiers_manager.h>

using namespace Mash;


SUITE(ClassifiersManagerSuite)
{
    TEST(LoadingOfSimpleClassifierByName)
    {
        ClassifiersManager manager("classifiers");
        
        Classifier* pClassifier = manager.create("unittests/onefeature");
        CHECK(pClassifier);
        
        delete pClassifier;
    }
    

    TEST(LoadingOfAdvancedClassifierByName)
    {
        ClassifiersManager manager("classifiers");
        
        Classifier* pClassifier = manager.create("mash/perceptron");
        CHECK(pClassifier);
        
        delete pClassifier;
    }
    

    TEST(LoadingOfUnknownClassifierFail)
    {
        ClassifiersManager manager("classifiers");
        
        Classifier* pClassifier = manager.create("unknown");
        CHECK(!pClassifier);
        CHECK_EQUAL(ERROR_CLASSIFIER_LOADING, manager.getLastError());
    }


    TEST(LoadingOfClassifierLackingAConstructorFail)
    {
        ClassifiersManager manager("classifiers");
        
        Classifier* pClassifier = manager.create("unittests/noconstructor");
        CHECK(!pClassifier);
        CHECK_EQUAL(ERROR_CLASSIFIER_CONSTRUCTOR, manager.getLastError());
    }
}

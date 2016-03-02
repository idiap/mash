#include <mash-classification/sandboxed_classifier.h>
#include <iostream>
#include <string>
#include "tests.h"

using namespace Mash;
using namespace std;


int main(int argc, char** argv)
{
    SandboxedClassifier sandbox;
    tSandboxConfiguration configuration;
    
    configuration.strScriptsDir = MASH_SOURCE_DIR "sandbox/";
    configuration.strSourceDir  = MASH_SOURCE_DIR "classifiers/";
    configuration.strUsername  = MASH_TESTS_SANDBOX_USERNAME;
    
    CHECK(sandbox.createSandbox(configuration));
    
    CHECK(sandbox.setClassifiersFolder("classifiers"));

    CHECK(!sandbox.loadClassifierPlugin("unittests/noconstructor"));
    CHECK_EQUAL(ERROR_CLASSIFIER_CONSTRUCTOR, sandbox.getLastError());
    
    return 0;
}

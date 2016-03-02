#include <UnitTest++.h>
#include <mash/predictor_model.h>
#include <unistd.h>

using namespace Mash;
using namespace std;


const char* MODELFILE = "tmp_model.txt";


struct Environment
{
    Environment()
    {
        remove(MODELFILE);
    }
    
    ~Environment()
    {
        remove(MODELFILE);
    }
};


SUITE(PredictorModelSuite)
{
    TEST_FIXTURE(Environment, EmptyModelHasNoHeuristic)
    {
        PredictorModel model;
    
        CHECK_EQUAL(0, model.nbHeuristics());
    }


    TEST_FIXTURE(Environment, EmptyModelDontReportHeuristicIndex)
    {
        PredictorModel model;
    
        CHECK_EQUAL(-1, model.toModel(0));
    }
    
    
    TEST_FIXTURE(Environment, CreatedModel_isWritable)
    {
        PredictorModel model;

        CHECK(model.create(MODELFILE));
        CHECK(model.isWritable());
        CHECK(!model.isReadable());
        model.close();
    }


    TEST_FIXTURE(Environment, CreatedModel_toModel)
    {
        PredictorModel model;

        CHECK(model.create(MODELFILE));

        model.addHeuristic(0, "U/A", 1000);
        model.addHeuristic(2, "U/B", 2000);
        model.addHeuristic(20, "U/E", 5000);
        model.addHeuristic(10, "U/D", 4000);
        model.addHeuristic(5, "U/C", 3000);
        
        CHECK_EQUAL(5, model.nbHeuristics());
        CHECK_EQUAL(0, model.toModel(0));
        CHECK_EQUAL(1, model.toModel(2));
        CHECK_EQUAL(2, model.toModel(20));
        CHECK_EQUAL(3, model.toModel(10));
        CHECK_EQUAL(4, model.toModel(5));
        CHECK_EQUAL(-1, model.toModel(100));

        model.close();
    }


    TEST_FIXTURE(Environment, CreatedModel_fromModel)
    {
        PredictorModel model;

        CHECK(model.create(MODELFILE));

        model.addHeuristic(0, "U/A", 1000);
        model.addHeuristic(2, "U/B", 2000);
        model.addHeuristic(20, "U/E", 5000);
        model.addHeuristic(10, "U/D", 4000);
        model.addHeuristic(5, "U/C", 3000);
        
        CHECK_EQUAL(5, model.nbHeuristics());
        CHECK_EQUAL(0, model.fromModel(0));
        CHECK_EQUAL(2, model.fromModel(1));
        CHECK_EQUAL(20, model.fromModel(2));
        CHECK_EQUAL(10, model.fromModel(3));
        CHECK_EQUAL(5, model.fromModel(4));

        model.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_isReadable)
    {
        PredictorModel model1, model2;

        CHECK(model1.create(MODELFILE));
        model1.lockHeuristics();
        model1.close();

        CHECK(model2.open(MODELFILE));
        CHECK(model2.isReadable());
        CHECK(!model2.isWritable());
        model2.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_toModel)
    {
        PredictorModel model1, model2;
    
        CHECK(model1.create(MODELFILE));

        model1.addHeuristic(0, "U/A", 1000);
        model1.addHeuristic(2, "U/B", 2000);
        model1.addHeuristic(20, "U/E", 5000);
        model1.addHeuristic(10, "U/D", 4000);
        model1.addHeuristic(5, "U/C", 3000);
        
        CHECK(model1.lockHeuristics());

        model1.close();
    
        CHECK(model2.open(MODELFILE));

        model2.addHeuristic(100, "U/A");
        model2.addHeuristic(101, "U/B");
        model2.addHeuristic(102, "U/E");
        model2.addHeuristic(103, "U/D");
        model2.addHeuristic(104, "U/C");
        model2.addHeuristic(105, "U/F");
        
        CHECK(model2.lockHeuristics());

        CHECK_EQUAL(5, model2.nbHeuristics());
        CHECK_EQUAL(0, model2.toModel(100));
        CHECK_EQUAL(1, model2.toModel(101));
        CHECK_EQUAL(2, model2.toModel(102));
        CHECK_EQUAL(3, model2.toModel(103));
        CHECK_EQUAL(4, model2.toModel(104));
        CHECK_EQUAL(-1, model2.toModel(105));

        model2.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_fromModel)
    {
        PredictorModel model1, model2;
    
        CHECK(model1.create(MODELFILE));

        model1.addHeuristic(0, "U/A", 1000);
        model1.addHeuristic(2, "U/B", 2000);
        model1.addHeuristic(20, "U/E", 5000);
        model1.addHeuristic(10, "U/D", 4000);
        model1.addHeuristic(5, "U/C", 3000);
        
        CHECK(model1.lockHeuristics());

        model1.close();
    
        CHECK(model2.open(MODELFILE));

        model2.addHeuristic(100, "U/A");
        model2.addHeuristic(101, "U/B");
        model2.addHeuristic(102, "U/E");
        model2.addHeuristic(103, "U/D");
        model2.addHeuristic(104, "U/C");
        model2.addHeuristic(105, "U/F");
        
        CHECK(model2.lockHeuristics());

        CHECK_EQUAL(5, model2.nbHeuristics());
        CHECK_EQUAL(100, model2.fromModel(0));
        CHECK_EQUAL(101, model2.fromModel(1));
        CHECK_EQUAL(102, model2.fromModel(2));
        CHECK_EQUAL(103, model2.fromModel(3));
        CHECK_EQUAL(104, model2.fromModel(4));

        model2.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_missingHeuristics)
    {
        PredictorModel model1, model2;
    
        CHECK(model1.create(MODELFILE));

        model1.addHeuristic(0, "U/A", 1000);
        model1.addHeuristic(2, "U/B", 2000);
        model1.addHeuristic(20, "U/E", 5000);
        model1.addHeuristic(10, "U/D", 4000);
        model1.addHeuristic(5, "U/C", 3000);
        
        CHECK(model1.lockHeuristics());

        model1.close();
    
        CHECK(model2.open(MODELFILE));

        model2.addHeuristic(100, "U/A");
        model2.addHeuristic(105, "U/F");
        
        CHECK(!model2.lockHeuristics());

        CHECK_EQUAL(5, model2.nbHeuristics());
        CHECK_EQUAL(100, model2.fromModel(0));
        CHECK_EQUAL(-1, model2.fromModel(1));
        CHECK_EQUAL(-1, model2.fromModel(2));
        CHECK_EQUAL(-1, model2.fromModel(3));
        CHECK_EQUAL(-1, model2.fromModel(4));

        tStringList missing = model2.missingHeuristics();
        CHECK_EQUAL("U/B/1", missing[0]);
        CHECK_EQUAL("U/E/1", missing[1]);
        CHECK_EQUAL("U/D/1", missing[2]);
        CHECK_EQUAL("U/C/1", missing[3]);
        
        model2.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_predictorSeed)
    {
        PredictorModel model1, model2;
    
        CHECK(model1.create(MODELFILE));

        model1.setPredictorSeed(1000);
        
        CHECK(model1.lockHeuristics());

        model1.close();
    
        CHECK(model2.open(MODELFILE));

        CHECK(model2.lockHeuristics());

        CHECK_EQUAL(1000, model2.predictorSeed());

        model2.close();
    }


    TEST_FIXTURE(Environment, LoadedModel_heuristicsSeed)
    {
        PredictorModel model1, model2;
    
        CHECK(model1.create(MODELFILE));

        model1.addHeuristic(0, "U/A", 1000);
        model1.addHeuristic(2, "U/B", 2000);
        model1.addHeuristic(20, "U/E", 5000);
        model1.addHeuristic(10, "U/D", 4000);
        model1.addHeuristic(5, "U/C", 3000);
        
        CHECK(model1.lockHeuristics());

        model1.close();
    
        CHECK(model2.open(MODELFILE));

        model2.addHeuristic(100, "U/A");
        model2.addHeuristic(101, "U/B");
        model2.addHeuristic(102, "U/E");
        model2.addHeuristic(103, "U/D");
        model2.addHeuristic(104, "U/C");
        model2.addHeuristic(105, "U/F");
        
        CHECK(model2.lockHeuristics());

        CHECK_EQUAL(5, model2.nbHeuristics());
        CHECK_EQUAL(1000, model2.heuristicSeed("U/A"));
        CHECK_EQUAL(2000, model2.heuristicSeed("U/B"));
        CHECK_EQUAL(5000, model2.heuristicSeed("U/E"));
        CHECK_EQUAL(4000, model2.heuristicSeed("U/D"));
        CHECK_EQUAL(3000, model2.heuristicSeed("U/C"));
        CHECK_EQUAL(0, model2.heuristicSeed("U/F"));

        model2.close();
    }
}

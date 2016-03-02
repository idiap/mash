#include <mash-classification/classifier.h>

using namespace Mash;


int crash()
{
    int* pointer = 0;
    *pointer = 4;
    
    return 0;
}


static int A = crash();



class CrashClassifier: public Classifier
{
    //_____ Construction / Destruction __________
public:
    CrashClassifier()
    {
    }
    
    virtual ~CrashClassifier()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        return true;
    }

    virtual bool loadModel(PredictorModel &model, DataReader &internal_data)
    {
        return true;
    }

    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error)
    {
        return true;
    }

    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results)
    {
        return true;
    }
    
    virtual bool reportFeaturesUsed(tFeatureList &list)
    {
        return true;
    }

    virtual bool saveModel(PredictorModel &model)
    {
        return true;
    }
};


extern "C" Classifier* new_classifier()
{
    return new CrashClassifier();
}

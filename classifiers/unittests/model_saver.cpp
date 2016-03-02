#include <mash-classification/classifier.h>

using namespace Mash;
using namespace std;


class TestClassifier: public Classifier
{
    //_____ Construction / Destruction __________
public:
    TestClassifier()
    : _nbHeuristics(0), _bUseInternalData(false)
    {
    }
    
    virtual ~TestClassifier()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        tExperimentParametersIterator iter;

        iter = parameters.find("USE_INTERNAL_DATA");
        if (iter != parameters.end())
            _bUseInternalData = (iter->second.getInt(0) == 1);
        
        return true;
    }

    virtual bool loadModel(PredictorModel &model, DataReader &internal_data)
    {
        return true;
    }

    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error)
    {
        _nbHeuristics = input_set->nbHeuristics();
        
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
        for (unsigned int i = 0; i < _nbHeuristics; ++i)
            list.push_back(tFeature(i, 0));
        
        return true;
    }

    virtual bool saveModel(PredictorModel &model)
    {
        if (!model.isWritable())
            return false;

        if (_bUseInternalData && !outInternalData.isOpen())
            return false;

        if (model.nbHeuristics() != _nbHeuristics)
            return false;

        model.writer() << "OK" << endl;


        if (_bUseInternalData)
            outInternalData << "OK" << endl;

        return true;
    }


    // _____ Attributes __________
protected:
    unsigned int _nbHeuristics;
    bool _bUseInternalData;
};


extern "C" Classifier* new_classifier()
{
    return new TestClassifier();
}

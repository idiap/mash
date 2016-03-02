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

        iter = parameters.find("NB_HEURISTICS");
        if (iter != parameters.end())
            _nbHeuristics = iter->second.getInt(0);

        iter = parameters.find("USE_INTERNAL_DATA");
        if (iter != parameters.end())
            _bUseInternalData = (iter->second.getInt(0) == 1);
        
        return true;
    }

    virtual bool loadModel(PredictorModel &model, DataReader &internal_data)
    {
        if (!model.isReadable())
            return false;

        if (_bUseInternalData && !internal_data.isOpen())
            return false;

        if (model.nbHeuristics() != _nbHeuristics)
            return false;

        const int64_t BUFFER_SIZE = 50;
        char buffer[BUFFER_SIZE];

        if (model.reader().tell() != 0)
            return false;

        if (model.reader().readline(buffer, BUFFER_SIZE) <= 0)
            return false;

        if (string(buffer) != "OK")
            return false;

        if (!model.reader().eof())
            return false;


        if (_bUseInternalData)
        {
            if (internal_data.tell() != 0)
                return false;

            if (internal_data.readline(buffer, BUFFER_SIZE) <= 0)
                return false;

            if (string(buffer) != "OK")
                return false;

            if (!internal_data.eof())
                return false;
        }
                
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


    // _____ Attributes __________
protected:
    unsigned int _nbHeuristics;
    bool _bUseInternalData;
};


extern "C" Classifier* new_classifier()
{
    return new TestClassifier();
}

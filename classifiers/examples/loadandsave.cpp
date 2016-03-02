#include <mash-classification/classifier.h>

using namespace Mash;
using namespace std;


class LoadAndSave: public Classifier
{
    //_____ Construction / Destruction __________
public:
    LoadAndSave();
    virtual ~LoadAndSave();


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters);
    virtual bool loadModel(PredictorModel &model, DataReader &internal_data);
    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);
    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results);
    virtual bool reportFeaturesUsed(tFeatureList &list);
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:
    tExperimentParametersList   parameters;
    PredictorModel              model;
    DataReader                  internal_data;
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new LoadAndSave();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

LoadAndSave::LoadAndSave()
{
}


LoadAndSave::~LoadAndSave()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool LoadAndSave::setup(const tExperimentParametersList& parameters)
{
    outStream << "setup()" << endl;
    
    // Copy for later use
    this->parameters = parameters;

    return true;
}


bool LoadAndSave::loadModel(PredictorModel &model, DataReader &internal_data)
{
    outStream << "loadModel()" << endl;
    
    // Copy for later use
    this->model = model;
    this->internal_data = internal_data;

    return true;
}


bool LoadAndSave::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    outStream << "train()" << endl;
    
    if (model.isReadable())
    {
        outStream << "    -> adaptation of the model" << endl;

        // TODO: Use parameters, model and internal_data

        model.close();
        internal_data.close();
    }
    else
    {
        outStream << "    -> training of a new model" << endl;

        // TODO: Use parameters
    }
    
    // To prevent the Framework to compute the train error since we know it
    train_error = 0.2f;
        
    return true;
}


bool LoadAndSave::classify(IClassifierInputSet* input_set,
                           unsigned int image,
                           const coordinates_t& position,
                           tClassificationResults &results)
{
    outStream << "classify()" << endl;

    if (model.isReadable())
    {
        outStream << "    -> model not adapted (no call to train()), loading it" << endl;

        // TODO: Use parameters, model and internal_data

        model.close();
        internal_data.close();
    }

    outStream << "    -> classification of image " << image << endl;

    results[0] = 1.0f;

    return true;
}


bool LoadAndSave::reportFeaturesUsed(tFeatureList &list)
{
    outStream << "reportFeaturesUsed()" << endl;

    // Let's say we only use one feature
    list.push_back(tFeature(0, 0));

    return true;
}


bool LoadAndSave::saveModel(PredictorModel &model)
{
    outStream << "saveModel()" << endl;

    // TODO: Save the model

    return true;
}

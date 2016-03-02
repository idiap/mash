/** Author: YOUR_USERNAME

    TODO: Write a description of your classifier
*/

#include <mash-classification/classifier.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the classifier class
//
// TODO: Change the names of the class, the constructor and the destructor. Also
//       change the name of the class in the implementation of each method below
//------------------------------------------------------------------------------
class MyClassifier: public Classifier
{
    //_____ Construction / Destruction __________
public:
    MyClassifier();
    virtual ~MyClassifier();


    //_____ Methods to implement __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Initialize the classifier
    ///
    /// @param  parameters  The classifier-specific parameters
    /// @return             'true' if successful
    //--------------------------------------------------------------------------
    virtual bool setup(const tExperimentParametersList& parameters);

    //--------------------------------------------------------------------------
    /// @brief  Load a model
    ///
    /// @param  model           The model object to use
    /// @param  internal_data   (Optional) Used to retrieve classifier-specific
    ///                         data saved alongside the model
    /// @return                 'true' if successful
    ///
    /// @note   This method will be called after setup(), but only if an
    ///         already trained model must be used. Then, either we begin
    ///         directly with calls to classify(), or the classifier is given
    ///         the opportunity to adapt its model via a call to train().
    ///
    /// The provided model has already been initialized (ie. it already
    /// knows about the heuristics used by the classifier that saved it).
    ///
    /// In some specific scenarios, when saving a model, the classifier can
    /// also save additional data. The goal here is to speed-up subsequent
    /// experiments using the saved model. However, this internal data might
    /// not always be provided alongside the model, so the classifier must
    /// be able to setup itself using only the model (ie. if
    /// internal_data.isOpen() == false, the internal data isn't available).
    //--------------------------------------------------------------------------
    virtual bool loadModel(PredictorModel &model, DataReader &internal_data);

    //--------------------------------------------------------------------------
    /// @brief  Train the classifier
    ///
    /// @param      input_set   The Classifier Input Set to use
    /// @param[out] train_error (Optional) The train error. Leave unchanged
    ///                         if you want to let the Framework compute it.
    /// @return                 'true' if successful
    //--------------------------------------------------------------------------
    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);

    //--------------------------------------------------------------------------
    /// @brief  Classify the content of an image at a specific position
    ///
    /// @param  input_set   The Classifier Input Set to use
    /// @param  image       Index of the image (in the Input Set)
    /// @param  position    Center of the region-of-interest (in the image)
    /// @retval results     Classification results
    /// @return             'true' if successful
    //--------------------------------------------------------------------------
    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results);

    //--------------------------------------------------------------------------
    /// @brief  Populates the provided list with the features used by the
    ///         classifier
    ///
    /// @retval list    The list of features used
    /// @return         'true' if successful
    ///
    /// @note   This method will only be called after train(), and the 'list' 
    ///         parameter will always be empty.
    //--------------------------------------------------------------------------
    virtual bool reportFeaturesUsed(tFeatureList &list);

    //--------------------------------------------------------------------------
    /// @brief  Save the model trained by the classifier
    ///
    /// @param  model   The model object to use
    /// @return         'true' if successful
    ///
    /// @note   This method will only be called at the end of the experiment.
    ///
    /// The provided model has already been initialized (ie. it already
    /// knows about the heuristics used by the classifier, reported by
    /// reportFeaturesUsed()).
    ///
    /// In some specific scenarios, the classifier can save whatever it wants
    /// using the attribute 'outInternalData'. The goal here is to speed-up
    /// subsequent experiments using the saved model. However, this internal
    /// data might not always be provided alongside the model, so the
    /// classifier must be able to setup itself using only the model.
    //--------------------------------------------------------------------------
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:
    // TODO: Declare all the attributes you'll need here
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//
// TODO: Change the name of the instanciated class
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new MyClassifier();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

MyClassifier::MyClassifier()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


MyClassifier::~MyClassifier()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool MyClassifier::setup(const tExperimentParametersList& parameters)
{
    // TODO: Implement it
    return true;
}


bool MyClassifier::loadModel(PredictorModel &model, DataReader &internal_data)
{
    // TODO: Implement it
    return true;
}


bool MyClassifier::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // TODO: Implement it
    return true;
}


bool MyClassifier::classify(IClassifierInputSet* input_set,
                            unsigned int image,
                            const coordinates_t& position,
                            tClassificationResults &results)
{
    // TODO: Implement it
    return true;
}


bool MyClassifier::reportFeaturesUsed(tFeatureList &list)
{
    // TODO: Implement it
    return true;
}


bool MyClassifier::saveModel(PredictorModel &model)
{
    // TODO: Implement it
    return true;
}

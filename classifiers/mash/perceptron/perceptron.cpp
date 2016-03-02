/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    Example classifier for image classification problems. It trains one
    linear classifier per image label, using the perceptron rule.
*/

#include <mash-classification/classifier.h>
#include <mash-utils/stringutils.h>
#include "linear_classifier.h"

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the 'Perceptron' classifier
//
// See the wiki of the project for an in-depth explanation of this example
//------------------------------------------------------------------------------
class Perceptron: public Classifier
{
    //_____ Construction / Destruction __________
public:
    Perceptron();
    virtual ~Perceptron();


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
    LinearClassifier*   linearClassifiers;
    unsigned int        nbLinearClassifiers;
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new Perceptron();
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

Perceptron::Perceptron()
: linearClassifiers(0), nbLinearClassifiers(0)
{
}


Perceptron::~Perceptron()
{
    // Release the memory
    delete[] linearClassifiers;
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Perceptron::setup(const tExperimentParametersList& parameters)
{
    tExperimentParametersIterator iter;
    
    // Retrieve the maximum number of features to use per heuristic
    iter = parameters.find("NB_MAX_FEATURES_PER_HEURISTIC");
    if (iter != parameters.end())
    {
        LinearClassifier::nbMaxFeaturesPerHeuristic = iter->second.getInt(0);
        if (LinearClassifier::nbMaxFeaturesPerHeuristic == 0)
            return false;
    }

    // Retrieve the maximum number of steps of the perceptron algorithm when
    // training a new model
    iter = parameters.find("NB_MAX_TRAINING_STEPS");
    if (iter != parameters.end())
    {
        LinearClassifier::nbMaxTrainingSteps = iter->second.getInt(0);
        if (LinearClassifier::nbMaxTrainingSteps == 0)
            return false;
    }

    // Retrieve the maximum number of steps of the perceptron algorithm when
    // adapating an existing model
    iter = parameters.find("NB_MAX_ADAPTATION_STEPS");
    if (iter != parameters.end())
    {
        LinearClassifier::nbMaxAdaptationSteps = iter->second.getInt(0);
        if (LinearClassifier::nbMaxAdaptationSteps == 0)
            return false;
    }

    // Retrieve the maximum size of the cache used to keep features in memory
    // to speed-up the training
    iter = parameters.find("MAX_CACHE_SIZE");
    if (iter != parameters.end())
    {
        LinearClassifier::maxCacheSize = iter->second.getInt(0);
        if (LinearClassifier::maxCacheSize == 0)
            return false;
    }

    outStream << "Configuration:" << endl
              << "    - NB_MAX_FEATURES_PER_HEURISTIC: " << LinearClassifier::nbMaxFeaturesPerHeuristic << endl
              << "    - NB_MAX_TRAINING_STEPS:         " << LinearClassifier::nbMaxTrainingSteps << endl
              << "    - NB_MAX_ADAPTATION_STEPS:       " << LinearClassifier::nbMaxAdaptationSteps << endl
              << "    - MAX_CACHE_SIZE:                " << LinearClassifier::maxCacheSize << endl;

    return true;
}


bool Perceptron::loadModel(PredictorModel &model, DataReader &internal_data)
{
    // Note: See saveModel() to have details about the format of the model file
    
    // Declarations
    LinearClassifier*                   pCurrentLinearClassifier = 0;
    LinearClassifier::tHeuristicEntry*  pCurrentHeuristic        = 0;
    unsigned int                        currentFeature           = 0;
    char                                buffer[50];

    outStream << "Loading a model..." << endl;

    while (!model.reader().eof())
    {
        // Read the next line of the model
        if (model.reader().readline(buffer, 50) <= 0)
        {
            outStream << "    ERROR - Failed to read a line from the model file" << endl;
            return false;
        }

        // Don't care about the comments
        if (buffer[0] == '#')
            continue;

        // Split the line of data
        tStringList parts = StringUtils::split(buffer, " ");
        
        // Read the number of classifiers to create (if not done already)
        if (!linearClassifiers)
        {
            if ((parts[0] != "NB_CLASSIFIERS") || (parts.size() != 2))
            {
                outStream << "    ERROR - Can't retrieve the number of classifiers: " << buffer << endl;
                return false;
            }

            // Create the linear classifiers
            nbLinearClassifiers = StringUtils::parseUnsignedInt(parts[1]);
            linearClassifiers   = new LinearClassifier[nbLinearClassifiers];
        }
        
        // Test if we are at the beginning of a new linear classifier
        else if (parts[0] == "CLASSIFIER")
        {
            if (parts.size() != 3)
            {
                outStream << "    ERROR - Can't retrieve the label: " << buffer << endl;
                return false;
            }

            // Initialize it
            unsigned int label        = StringUtils::parseUnsignedInt(parts[1]);
            unsigned int nbHeuristics = StringUtils::parseUnsignedInt(parts[2]);

            if (!pCurrentLinearClassifier)
                pCurrentLinearClassifier = linearClassifiers;
            else
                ++pCurrentLinearClassifier;

            pCurrentLinearClassifier->setup(label, notifier, outStream);
            pCurrentLinearClassifier->_modelSize = nbHeuristics;
            pCurrentLinearClassifier->_model = new LinearClassifier::tHeuristicEntry[nbHeuristics];

            pCurrentHeuristic = 0;
        }

        // Test if the line contain informations about a heuristic
        else if (parts[0] == "HEURISTIC")
        {
            if (parts.size() != 3)
            {
                outStream << "    ERROR - Can't retrieve the heuristic: " << buffer << endl;
                return false;
            }

            if (!pCurrentLinearClassifier)
            {
                outStream << "    ERROR - No current linear classifier" << endl;
                return false;
            }

            // Initialize it
            unsigned int heuristic  = StringUtils::parseUnsignedInt(parts[1]);
            unsigned int nbFeatures = StringUtils::parseUnsignedInt(parts[2]);

            if (!pCurrentHeuristic)
                pCurrentHeuristic = pCurrentLinearClassifier->_model;
            else
                ++pCurrentHeuristic;

            // Convert the heuristic index from the model-space before using it
            pCurrentHeuristic->heuristic = model.fromModel(heuristic);
            pCurrentHeuristic->nbFeatures = nbFeatures;
            pCurrentHeuristic->features = new unsigned int[nbFeatures];
            pCurrentHeuristic->weights = new scalar_t[nbFeatures];
            
            currentFeature = 0;
        }

        // Test if the line contain informations about a feature
        else if (parts[0] == "FEATURE")
        {
            if (parts.size() != 3)
            {
                outStream << "    ERROR - Can't retrieve the feature: " << buffer << endl;
                return false;
            }

            if (!pCurrentLinearClassifier)
            {
                outStream << "    ERROR - No current linear classifier" << endl;
                return false;
            }

            if (!pCurrentHeuristic)
            {
                outStream << "    ERROR - No current heuristic" << endl;
                return false;
            }

            // Initialize it
            pCurrentHeuristic->features[currentFeature] = StringUtils::parseUnsignedInt(parts[1]);
            pCurrentHeuristic->weights[currentFeature]  = StringUtils::parseFloat(parts[2]);
            
            ++currentFeature;
        }
    }

    return true;
}


bool Perceptron::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // Determine if we must train a new model or adapt an existing one
    if (!linearClassifiers)
    {
        //_____ Training of a new model __________

        // Check that there is at least two labels
        if (input_set->nbLabels() < 2)
            return false;
    
        // Create one linear classifier per label, unless there is only two labels
        if (input_set->nbLabels() == 2)
            nbLinearClassifiers = 1;
        else
            nbLinearClassifiers = input_set->nbLabels();

        linearClassifiers = new LinearClassifier[nbLinearClassifiers];

        // Send a notification
        notifier.onTrainingStepDone(0, LinearClassifier::nbMaxTrainingSteps * input_set->nbLabels());

        // Initialize the linear classifiers and train them
        for (unsigned int i = 0; i < nbLinearClassifiers; ++i)
        {
            linearClassifiers[i].setup(i, notifier, outStream);
        
            if (!linearClassifiers[i].train(input_set, generator.randomize()))
                return false;
        }
    }
    else
    {
        //_____ Adaptation __________

        // Send a notification
        notifier.onTrainingStepDone(0, LinearClassifier::nbMaxAdaptationSteps * input_set->nbLabels());

        // Adapt each linear classifier
        for (unsigned int i = 0; i < nbLinearClassifiers; ++i)
        {
            if (!linearClassifiers[i].adapt(input_set, generator.randomize()))
                return false;
        }
    }

    return true;
}


bool Perceptron::classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results)
{
    // Ask all the linear classifiers
    for (unsigned int i = 0; i < nbLinearClassifiers; ++i)
    {
        scalar_t score;
        
        if (!linearClassifiers[i].classify(input_set, image, position, score))
            return false;
    
        results[i] = score;
    }
    
    // If we only have one classifier (-> only two labels), negate the score of
    // the first one to make the second one
    if (nbLinearClassifiers == 1)
        results[1] = -results[0];

    return true;
}


bool Perceptron::reportFeaturesUsed(tFeatureList &list)
{
    // Retrieves the features used by each linear classifier
    // Note: we don't have to remove duplicates, the Framework will take care of
    // that for us
    for (unsigned int i = 0; i < nbLinearClassifiers; ++i)
        linearClassifiers[i].reportFeaturesUsed(list);

    return true;
}


bool Perceptron::saveModel(PredictorModel &model)
{
    // Note: Here we save the model in a human-readable format, but it can be
    // done in binary form too. The format of the content of the model file is
    // totally up to the classifier (besides a standard header added by the
    // Framework).

    // First output some comments, to let anyone know how to interpret the
    // content
    model.writer() << "# Format 1.0" << endl
                   << "# ---------------" << endl
                   << "# NB_CLASSIFIERS <nb_classifiers>" << endl
                   << "# CLASSIFIER <label> <nb_heuristics>" << endl
                   << "# HEURISTIC <heuristic> <nb_features>" << endl
                   << "# FEATURE <feature> <weight>" << endl
                   << "# FEATURE <feature> <weight>" << endl
                   << "# ..." << endl
                   << "# ---------------" << endl;

    model.writer() << "NB_CLASSIFIERS " << nbLinearClassifiers << endl;

    // Save the features used by each linear classifier
    for (unsigned int i = 0; i < nbLinearClassifiers; ++i)
    {
        model.writer() << "CLASSIFIER " << linearClassifiers[i]._label << " "
                       << linearClassifiers[i]._modelSize << endl;

        for (unsigned int j = 0; j < linearClassifiers[i]._modelSize; ++j)
        {
            LinearClassifier::tHeuristicEntry* pEntry = &linearClassifiers[i]._model[j];

            // Note: We must convert the index of the heuristic to the
            // model-space before using it in the model
            model.writer() << "HEURISTIC " << model.toModel(pEntry->heuristic) << " "
                           << pEntry->nbFeatures << endl;

            for (unsigned int k = 0; k < pEntry->nbFeatures; ++k)
            {
                model.writer() << "FEATURE " << pEntry->features[k] << " "
                               << pEntry->weights[k] << endl;
            }
        }
    }

    outStream << "Model saved" << endl;

    return true;
}

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


/** @file   classifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Classifier' interface
*/

#ifndef _MASH_CLASSIFIER_H_
#define _MASH_CLASSIFIER_H_

#include "declarations.h"
#include "classifier_input_set_interface.h"
#include <mash-utils/outstream.h>
#include <mash-utils/random_number_generator.h>
#include <mash-utils/data_writer.h>
#include <mash-utils/data_reader.h>
#include <mash/predictor_model.h>
#include <mash/notifier.h>
#include <map>
#include <vector>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for the classifiers
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Classifier
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Contains the results of the classification
        ///
        /// Usage: results[<the_label>] = <the_score>;
        /// Example (with the 3 classes 0, 1 and -1):
        /// @code
        ///    results[0] = 0.98f;
        ///    results[1] = 0.54f;
        ///    results[-1] = 0.1f;
        /// @endcode
        ///
        /// @remark The classifiers don't have to provide a score for each
        ///         existing label, only the ones it thinks to be relevant.
        //----------------------------------------------------------------------
        typedef std::map<int, scalar_t> tClassificationResults;


        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Classifier() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Classifier() {}


        //_____ Methods to implement __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Initialize the classifier
        ///
        /// @param  parameters  The classifier-specific parameters
        /// @return             'true' if successful
        ///
        /// @note   The 'tExperimentParametersList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual bool setup(const tExperimentParametersList& parameters) = 0;

        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
        virtual bool loadModel(PredictorModel &model, DataReader &internal_data) = 0;

        //----------------------------------------------------------------------
        /// @brief  Train the classifier
        ///
        /// @param      input_set   The Classifier Input Set to use
        /// @param[out] train_error (Optional) The train error. Leave unchanged
        ///                         if you want to let the Framework compute it.
        /// @return                 'true' if successful
        //----------------------------------------------------------------------
        virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error) = 0;

        //----------------------------------------------------------------------
        /// @brief  Classify the content of an image at a specific position
        ///
        /// @param  input_set   The Classifier Input Set to use
        /// @param  image       Index of the image (in the Input Set)
        /// @param  position    Center of the region-of-interest (in the image)
        /// @retval results     Classification results
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool classify(IClassifierInputSet* input_set,
                              unsigned int image,
                              const coordinates_t& position,
                              tClassificationResults &results) = 0;

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the features used by the
        ///         classifier
        ///
        /// @retval list    The list of features used
        /// @return         'true' if successful
        ///
        /// @note   This method will only be called after train(), and the
        ///         'list' parameter will always be empty.
        ///
        /// @note   The 'tFeatureList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual bool reportFeaturesUsed(tFeatureList &list) = 0;

        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
        virtual bool saveModel(PredictorModel &model) = 0;


        //_____ Attributes __________
    public:
        OutStream               outStream;          ///< The stream to use for logging
        RandomNumberGenerator   generator;          ///< The random number generator to use
        Notifier                notifier;           ///< The object to use to send notifications
        DataWriter              outFeatureCache;    ///< The object to use to cache features
        DataReader              inFeatureCache;     ///< The object to use to retrieve cached features
        DataWriter              writer;             ///< The object to use to save data to be
                                                    ///  analyzed later
        DataWriter              outInternalData;    ///< See the documentation of saveModel()
    };


    //--------------------------------------------------------------------------
    /// @brief  Prototype of the function used to create an instance of a
    ///         specific classifier object
    ///
    /// Each classifier must have an associated function called 'new_classifier'
    /// that creates an instance of the classifier. For instance, if your
    /// classifier class is 'MyClassifier', your 'new_classifier' function must be:
    /// @code
    /// extern "C" Classifier* new_classifier()
    /// {
    ///     return new MyClassifier();
    /// }
    /// @endcode
    //--------------------------------------------------------------------------
    typedef Classifier* tClassifierConstructor();
}

#endif

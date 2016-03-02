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


/** @file   classifier_delegate.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IClassifierDelegate' interface
*/

#ifndef _MASH_ICLASSIFIERDELEGATE_H_
#define _MASH_ICLASSIFIERDELEGATE_H_

#include "declarations.h"
#include "classifier.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface that must be implemented by the classes managing a
    ///         classifier
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IClassifierDelegate
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        IClassifierDelegate() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IClassifierDelegate() {}


        //_____ Classifier management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the delegate about the folder into which the classifiers
        ///         plugins are located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setClassifiersFolder(const std::string& strPath) = 0;

        //----------------------------------------------------------------------
        /// @brief  Tell the delegate load a specific classifier plugin and
        ///         create an instance of corresponding classifier
        ///
        /// @param  strName             Name of the plugin
        /// @param  strModelFile        (Optional) Path to the model file to
        ///                             load
        /// @param  strInternalDataFile (Optional) Path to the internal data
        ///                             file to load
        /// @return                     'true' if successful
        //----------------------------------------------------------------------
        virtual bool loadClassifierPlugin(const std::string& strName,
                                          const std::string& strModelFile = "",
                                          const std::string& strInternalDataFile = "") = 0;

        //----------------------------------------------------------------------
        /// @brief  Sets the notifier object to use
        //----------------------------------------------------------------------
        virtual void setNotifier(INotifier* pNotifier) = 0;


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the initial seed that must be used by the classifier
        ///
        /// @param  seed        The initial seed
        //----------------------------------------------------------------------
        virtual bool setSeed(unsigned int seed) = 0;
        
        //----------------------------------------------------------------------
        /// @brief  Initialize the classifier
        ///
        /// @param  parameters  The classifier-specific parameters
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setup(const tExperimentParametersList& parameters) = 0;

        //----------------------------------------------------------------------
        /// @brief  Load the model
        ///
        /// @param  input_set   The (read-only) Classifier Input Set to use to
        ///                     access the heuristics
        /// @return             'true' if successful
        ///
        /// @note   A model file must have been provided to the
        ///         loadClassifierPlugin() method
        //----------------------------------------------------------------------
        virtual bool loadModel(IClassifierInputSet* input_set) = 0;

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
                              Classifier::tClassificationResults &results) = 0;

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the features used by the
        ///         classifier
        ///
        /// @param  input_set   The (read-only) Classifier Input Set to use to
        ///                     access the heuristics
        /// @retval list        The list of features used
        /// @return             'true' if successful
        ///
        /// @note   This method will only be called after train(), and the
        ///         'list' parameter will always be empty.
        //----------------------------------------------------------------------
        virtual bool reportFeaturesUsed(IClassifierInputSet* input_set,
                                        tFeatureList &list) = 0;

        //----------------------------------------------------------------------
        /// @brief  Save the model trained by the classifier
        ///
        /// @return 'true' if successful
        ///
        /// @note   This method will only be called at the end of the experiment.
        ///         The model must be saved in the Data Report, with the name
        ///         'predictor.model'.
        //----------------------------------------------------------------------
        virtual bool saveModel() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        virtual tError getLastError() = 0;
    };
}

#endif

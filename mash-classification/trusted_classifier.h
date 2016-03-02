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


/** @file   trusted_classifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'TrustedClassifier' class
*/

#ifndef _MASH_TRUSTEDCLASSIFIER_H_
#define _MASH_TRUSTEDCLASSIFIER_H_

#include "declarations.h"
#include "classifier_delegate.h"
#include <mash-classification/classifiers_manager.h>
#include <mash-classification/classifier.h>
#include <mash/predictor_model.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a trusted classifier
    //--------------------------------------------------------------------------
    class MASH_SYMBOL TrustedClassifier: public IClassifierDelegate
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        TrustedClassifier();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~TrustedClassifier();


        //_____ Management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Configure the trusted classifier
        ///
        /// @param  strLogFolder    Folder where the log files must be written
        /// @param  strReportFolder Folder where the report files must be
        ///                         written
        //----------------------------------------------------------------------
        void configure(const std::string& strLogFolder,
                       const std::string& strReportFolder);


        //_____ Classifier management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the delegate about the folder into which the classifiers
        ///         plugins are located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setClassifiersFolder(const std::string& strPath);

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
                                          const std::string& strInternalDataFile = "");

        //----------------------------------------------------------------------
        /// @brief  Sets the notifier object to use
        //----------------------------------------------------------------------
        virtual void setNotifier(INotifier* pNotifier);


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the initial seed that must be used by the classifier
        ///
        /// @param  seed        The initial seed
        //----------------------------------------------------------------------
        virtual bool setSeed(unsigned int seed);
        
        //----------------------------------------------------------------------
        /// @brief  Initialize the classifier
        ///
        /// @param  parameters  The classifier-specific parameters
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setup(const tExperimentParametersList& parameters);

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
        virtual bool loadModel(IClassifierInputSet* input_set);

        //----------------------------------------------------------------------
        /// @brief  Train the classifier
        ///
        /// @param      input_set   The Classifier Input Set to use
        /// @param[out] train_error (Optional) The train error. Leave unchanged
        ///                         if you want to let the Framework compute it.
        /// @return                 'true' if successful
        //----------------------------------------------------------------------
        virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);

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
                              Classifier::tClassificationResults &results);

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the features used by the
        ///         classifier
        ///
        /// @param  input_set   The (read-only) Classifier Input Set to use to
        ///                     access the heuristics
        /// @return             'true' if successful
        ///
        /// You can safely assume that the method will only be called after
        /// train(), and that the 'list' parameter is empty.
        //----------------------------------------------------------------------
        virtual bool reportFeaturesUsed(IClassifierInputSet* input_set,
                                        tFeatureList &list);

        //----------------------------------------------------------------------
        /// @brief  Save the model trained by the classifier
        ///
        /// @return 'true' if successful
        ///
        /// @note   This method will only be called at the end of the experiment.
        ///         The model must be saved in the Data Report, with the name
        ///         'predictor.model'.
        //----------------------------------------------------------------------
        virtual bool saveModel();
        
        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        virtual tError getLastError();


        //_____ Attributes __________
    protected:
        ClassifiersManager* _pManager;          ///< The classifiers manager used
        Classifier*         _pClassifier;       ///< The classifier
        OutStream           _outStream;         ///< Output stream to use for logging
        tError              _lastError;         ///< Last error that occured
        PredictorModel      _inModel;           ///< The model to load
        DataReader          _inInternalData;    ///< Internal data associated to the model to load
        PredictorModel      _outModel;          ///< The model to save
        std::string         _strLogFolder;      ///< Path to the log folder
        std::string         _strReportFolder;   ///< Path to the Data Report folder
    };
}

#endif

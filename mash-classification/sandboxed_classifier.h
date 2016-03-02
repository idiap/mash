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


/** @file   sandboxed_classifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedClassifier' class
*/

#ifndef _MASH_SANDBOXEDCLASSIFIER_H_
#define _MASH_SANDBOXEDCLASSIFIER_H_

#include "declarations.h"
#include "classifier_delegate.h"
#include "sandbox_input_set_proxy.h"
#include <mash/sandbox_notifier_proxy.h>
#include <mash-sandboxing/sandbox_controller.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a sandboxed environment when untrusted classifiers
    ///         can be safely executed
    ///
    /// @remark Only one classifier can be loaded in the sandbox
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxedClassifier: public IClassifierDelegate, ISandboxControllerListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxedClassifier();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~SandboxedClassifier();


        //_____ Sandbox management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Create the sandbox
        ///
        /// @param  configuration   Configuration of the sandbox
        /// @return                 Error code
        //----------------------------------------------------------------------
        bool createSandbox(const tSandboxConfiguration& configuration);

        //----------------------------------------------------------------------
        /// @brief  Returns the Sandbox Controller used
        //----------------------------------------------------------------------
        inline SandboxController* sandboxController()
        {
            return &_sandbox;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Returns the context of the last operation
        //----------------------------------------------------------------------
        inline std::string getContext() const
        {
            return _strContext;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the stacktrace of the sandboxed object (used to
        ///         report debugging informations after a crash)
        //----------------------------------------------------------------------
        inline std::string getStackTrace()
        {
            return _sandbox.getStackTrace();
        }


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
        /// @param  input_set   The Classifier Input Set to use
        /// @return             'true' if successful
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
        /// @retval list        The list of features used
        /// @return             'true' if successful
        ///
        /// @note   This method will only be called after train(), and the
        ///         'list' parameter will always be empty.
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


        //_____ Implementation of ISandboxControllerListener __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when an unknown response was received by the sandbox
        ///         controller
        ///
        /// Since it might be the sandboxed object asking for some informations,
        /// this method will be called to handle it.
        ///
        /// @param  message     The response received by the sandbox controller
        /// @return             The result of the processing
        //----------------------------------------------------------------------
        virtual SandboxControllerDeclarations::tCommandProcessingResult
                    processResponse(tSandboxMessage message);


        //_____ Attributes __________
    protected:
        SandboxController       _sandbox;           ///< The sandbox used
        OutStream               _outStream;         ///< Output stream to use for logging
        SandboxInputSetProxy*   _pInputSetProxy;    ///< Proxy around the Input Set currently in use
        SandboxNotifierProxy*   _pNotifierProxy;    ///< Proxy around the Notifier currently in use
        std::string             _strContext;        ///< Context of the sandboxed object (used to report
                                                    ///  debugging informations after a crash)
    };
}

#endif

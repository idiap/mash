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


/** @file   sandboxed_instruments_set.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedInstrumentsSet' class
*/

#ifndef _MASH_SANDBOXEDINSTRUMENTSSET_H_
#define _MASH_SANDBOXEDINSTRUMENTSSET_H_

#include <mash-utils/declarations.h>
#include <mash-sandboxing/sandbox_controller.h>
#include <mash-sandboxing/declarations.h>
#include <mash-classification/sandbox_input_set_proxy.h>
#include <mash-goalplanning/sandbox_task_proxy.h>
#include "instruments_set_interface.h"
#include "instrument.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a sandboxed environment when untrusted instruments
    ///         can be safely executed
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxedInstrumentsSet: public IInstrumentsSet, ISandboxControllerListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxedInstrumentsSet();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~SandboxedInstrumentsSet();


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
        /// @brief  Returns the Sandboc Controller used
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


        //_____ Instruments management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the set about the folder into which the instrument
        ///         plugins are located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setInstrumentsFolder(const std::string& strPath);

        //----------------------------------------------------------------------
        /// @brief  Tell the set to load a specific instrument plugin
        ///
        /// @param  strName     Name of the plugin
        /// @return             The index of the plugin, -1 in case of error
        //----------------------------------------------------------------------
        virtual int loadInstrumentPlugin(const std::string& strName);

        //----------------------------------------------------------------------
        /// @brief  Tell the set to create instances of the instruments defined
        ///         in the loaded plugins
        ///
        /// @return 'true' if successful
        //----------------------------------------------------------------------
        virtual bool createInstruments();

        //----------------------------------------------------------------------
        /// @brief  Set the parameters of an instrument
        ///
        /// @param  instrument  Index of the instrument
        /// @param  parameters  The instrument-specific parameters
        /// @return             'true' if successful
        ///
        /// @note   The 'tExperimentParametersList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual bool setupInstrument(int instrument,
                                     const tExperimentParametersList& parameters);

        //----------------------------------------------------------------------
        /// @brief  Returns the number of instruments
        //----------------------------------------------------------------------
        virtual unsigned int nbInstruments() const;

        //----------------------------------------------------------------------
        /// @brief  Returns the index of an instrument
        ///
        /// @param  strName     Name of the instrument
        /// @return             The index of the instrument, -1 if not found
        //----------------------------------------------------------------------
        virtual int instrumentIndex(const std::string& strName) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the name of an instrument
        ///
        /// @param  index   Index of the instrument
        /// @return         The name of the instrument
        //----------------------------------------------------------------------
        virtual std::string instrumentName(int index) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the index of the current instrument
        //----------------------------------------------------------------------
        inline int currentInstrument() const
        {
            return _currentInstrument;
        }


        //_____ General events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called at the end of an experiment
        //----------------------------------------------------------------------
        virtual bool onExperimentDone();


        //_____ Classifier-related events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called at the beginning of a classification or object
        ///         detection experiment (right after everything is initialized)
        ///
        /// @param  input_set   The Input Set provided to the classifier
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onExperimentStarted(IClassifierInputSet* input_set);

        //----------------------------------------------------------------------
        /// @brief  Called when the training of the classifier has started (just
        ///         before calling Classifier::train())
        ///
        /// @param  input_set   The Input Set provided to the classifier
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onClassifierTrainingStarted(IClassifierInputSet* input_set);

        //----------------------------------------------------------------------
        /// @brief  Called when the training of the classifier is done (after
        ///         Classifier::train() has returned, and the performance of the
        ///         classifier was tested on the training set)
        ///
        /// @param  input_set   The Input Set provided to the classifier
        /// @param  train_error The train error
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onClassifierTrainingDone(IClassifierInputSet* input_set,
                                              scalar_t train_error);

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the classifier has started
        ///
        /// @param  input_set   The Input Set provided to the classifier
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onClassifierTestStarted(IClassifierInputSet* input_set);

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the classifier is done
        ///
        /// @param  input_set   The Input Set provided to the classifier
        /// @param  test_error  The test error
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onClassifierTestDone(IClassifierInputSet* input_set,
                                          scalar_t test_error);

        //----------------------------------------------------------------------
        /// @brief  Called when the classifier finished to classify an object
        ///
        /// @param  input_set       The Input Set provided to the classifier
        /// @param  image           Index of the image in the current set
        ///                         (training or test)
        /// @param  original_image  Original index of the image (in the database)
        /// @param  position        Position of the ROI/object
        /// @param  results         Classification results returned by the
        ///                         classifier
        /// @param  error           Indicates if the classifier did an error, and
        ///                         which one
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual bool onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                    unsigned int image,
                                                    unsigned int original_image,
                                                    const coordinates_t& position,
                                                    const Classifier::tClassificationResults& results,
                                                    Instrument::tClassificationError error);

        //----------------------------------------------------------------------
        /// @brief  Called when some features have been computed by a classifier
        ///
        /// @param  detection       Indicates if the experiment is an
        ///                         object-detection one
        /// @param  training        Indicates if we are in the training or test
        ///                         phase
        /// @param  image           Index of the image in the current set
        ///                         (training or test)
        /// @param  original_image  Original index of the image (in the database)
        /// @param  coords          Position of the ROI
        /// #param  roiExtent       Extent of the ROI
        /// @param  heuristic       Index of the heuristic
        /// @param  nbFeatures      Number of features computed
        /// @param  indexes         Indexes of the computed features
        /// @param  values          Values of the features
        //----------------------------------------------------------------------
        virtual bool onFeaturesComputedByClassifier(bool detection,
                                                    bool training,
                                                    unsigned int image,
                                                    unsigned int original_image,
                                                    const coordinates_t& coords,
                                                    unsigned int roiExtent,
                                                    unsigned int heuristic,
                                                    unsigned int nbFeatures,
                                                    unsigned int* indexes,
                                                    scalar_t* values);


        //_____ Goalplanner-related events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called at the beginning of a goal-planning experiment (right
        ///         after everything is initialized)
        ///
        /// @param  task    The task object provided to the goal-planner
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onExperimentStarted(ITask* task);

        //----------------------------------------------------------------------
        /// @brief  Called when the learning phase of the goal-planner has
        ///         started (just before calling Planner::learn())
        ///
        /// @param  task    The task object provided to the goal-planner
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onPlannerLearningStarted(ITask* task);

        //----------------------------------------------------------------------
        /// @brief  Called when the learning phase of the goal-planner (after
        ///         Planner::learn() has returned)
        ///
        /// @param  task    The task object provided to the goal-planner
        /// @param  result  Result of the task
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onPlannerLearningDone(ITask* task, tResult result);

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the goal-planner has started
        ///
        /// @param  task    The task object provided to the goal-planner
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onPlannerTestStarted(ITask* task);

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the goal-planner is done
        ///
        /// @param  task    The task object provided to the goal-planner
        /// @param  score   Score
        /// @param  result  Result of the task
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onPlannerTestDone(ITask* task, scalar_t score, tResult result);

        //----------------------------------------------------------------------
        /// @brief  Called when the goal-planner has choosed the next action to
        ///         perform
        ///
        /// @param  task    The task object provided to the goal-planner
        /// @param  action  The action choosed by the goal-planner
        /// @param  reward  Reward received after performing the action
        /// @param  result  Result of the task
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual bool onPlannerActionChoosen(ITask* task, unsigned int action,
                                            scalar_t reward, tResult result);

        //----------------------------------------------------------------------
        /// @brief  Called when some features have been computed by a
        ///         goal-planner
        ///
        /// @param  sequence    Index of the sequence
        /// @param  view        Index of the view
        /// @param  frame       Index of the frame
        /// @param  coords      Position of the ROI
        /// #param  roiExtent   Extent of the ROI
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features computed
        /// @param  indexes     Indexes of the computed features
        /// @param  values      Values of the features
        //----------------------------------------------------------------------
        virtual bool onFeaturesComputedByPlanner(unsigned int sequence,
                                                 unsigned int view,
                                                 unsigned int frame,
                                                 const coordinates_t& coords,
                                                 unsigned int roiExtent,
                                                 unsigned int heuristic,
                                                 unsigned int nbFeatures,
                                                 unsigned int* indexes,
                                                 scalar_t* values);


        //_____ Predictor-related events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when the list of features used by the predictor has
        ///         been retrieved
        ///
        /// @param  list    The list of features
        //----------------------------------------------------------------------
        virtual bool onFeatureListReported(const tFeatureList& features);


        //_____ Methods __________
    public:
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
        SandboxTaskProxy*       _pTaskProxy;        ///< Proxy around the Task or Perception currently in use
        int                     _currentInstrument;
        std::string             _strContext;        ///< Context of the sandboxed object (used to report
                                                    ///  debugging informations after a crash)
        tError                  _lastError;         ///< Last error that occured
    };
}

#endif

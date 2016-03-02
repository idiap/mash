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


/** @file   instrument.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Instrument' interface
*/

#ifndef _MASH_INSTRUMENT_H_
#define _MASH_INSTRUMENT_H_

#include <mash-utils/data_writer.h>
#include <mash-utils/declarations.h>
#include <mash-classification/classifier_input_set_interface.h>
#include <mash-classification/classifier.h>
#include <mash-goalplanning/task_interface.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for the instruments
    ///
    /// The goal of an instrument is to write some data in a file that will be
    /// analyzed later.
    ///
    /// It is expected that an instrument only use the data that is provided to
    /// him, and don't try to send commands to an Interactive Application Server
    /// or compute some features.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Instrument
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Enumerates the possible types of classification error
        //----------------------------------------------------------------------
        enum tClassificationError
        {
            CLASSIFICATION_ERROR_NONE,                  ///< No error
            CLASSIFICATION_ERROR_FALSE_ALARM,           ///< Detected a non-existant object
            CLASSIFICATION_ERROR_FALSE_REJECTION,       ///< Missed an object
            CLASSIFICATION_ERROR_WRONG_CLASSIFICATION,  ///< Classified an object with an incorrect label
        };


        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Instrument() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Instrument() {}


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Initialize the instrument
        ///
        /// @param  parameters  The instrument-specific parameters
        ///
        /// @note   The 'tExperimentParametersList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual void setup(const tExperimentParametersList& parameters) {}


        //_____ General events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called at the end of an experiment
        //----------------------------------------------------------------------
        virtual void onExperimentDone() {}


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
        virtual void onExperimentStarted(IClassifierInputSet* input_set) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the training of the classifier has started (just
        ///         before calling Classifier::train())
        ///
        /// @param  input_set   The Input Set provided to the classifier
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual void onClassifierTrainingStarted(IClassifierInputSet* input_set) {}

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
        virtual void onClassifierTrainingDone(IClassifierInputSet* input_set,
                                              scalar_t train_error) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the classifier has started
        ///
        /// @param  input_set   The Input Set provided to the classifier
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual void onClassifierTestStarted(IClassifierInputSet* input_set) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the classifier is done
        ///
        /// @param  input_set   The Input Set provided to the classifier
        /// @param  test_error  The test error
        ///
        /// @remark Only available for classification and object detection
        ///         experiments
        //----------------------------------------------------------------------
        virtual void onClassifierTestDone(IClassifierInputSet* input_set,
                                          scalar_t test_error) {}

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
        virtual void onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                    unsigned int image,
                                                    unsigned int original_image,
                                                    const coordinates_t& position,
                                                    const Classifier::tClassificationResults& results,
                                                    tClassificationError error) {}

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
        virtual void onFeaturesComputedByClassifier(bool detection,
                                                    bool training,
                                                    unsigned int image,
                                                    unsigned int original_image,
                                                    const coordinates_t& coords,
                                                    unsigned int roiExtent,
                                                    unsigned int heuristic,
                                                    unsigned int nbFeatures,
                                                    unsigned int* indexes,
                                                    scalar_t* values) {}


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
        virtual bool onExperimentStarted(ITask* task) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the learning phase of the goal-planner has
        ///         started (just before calling Planner::learn())
        ///
        /// @param  task    The task object provided to the goal-planner
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual void onPlannerLearningStarted(ITask* task) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the learning phase of the goal-planner (after
        ///         Planner::learn() has returned)
        ///
        /// @param  task    The task object provided to the goal-planner
        /// @param  result  Result of the task
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual void onPlannerLearningDone(ITask* task, tResult result) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the goal-planner has started
        ///
        /// @param  task    The task object provided to the goal-planner
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual void onPlannerTestStarted(ITask* task) {}

        //----------------------------------------------------------------------
        /// @brief  Called when the test of the goal-planner is done
        ///
        /// @param  task    The task object provided to the goal-planner
        /// @param  score   Score
        /// @param  result  Result of the task
        ///
        /// @remark Only available for goal-planning experiments
        //----------------------------------------------------------------------
        virtual void onPlannerTestDone(ITask* task, scalar_t score, tResult result) {}

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
        virtual void onPlannerActionChoosen(ITask* task, unsigned int action,
                                            scalar_t reward, tResult result) {}

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
        virtual void onFeaturesComputedByPlanner(unsigned int sequence,
                                                 unsigned int view,
                                                 unsigned int frame,
                                                 const coordinates_t& coords,
                                                 unsigned int roiExtent,
                                                 unsigned int heuristic,
                                                 unsigned int nbFeatures,
                                                 unsigned int* indexes,
                                                 scalar_t* values) {}


        //_____ Predictor-related events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when the list of features used by the predictor has
        ///         been retrieved
        ///
        /// @param  features    The list of features
        //----------------------------------------------------------------------
        virtual void onFeatureListReported(const tFeatureList& features) {}


        //_____ Attributes __________
    public:
        DataWriter writer;  ///< The object to use to save data
    };


    //--------------------------------------------------------------------------
    /// @brief  Prototype of the function used to create an instance of a
    ///         specific instrument object
    ///
    /// Each instrument must have an associated function called 'new_instrument'
    /// that creates an instance of the instrument. For instance, if your
    /// instrument class is 'MyInstrument', your 'new_instrument' function must be:
    /// @code
    /// extern "C" Instrument* new_instrument()
    /// {
    ///     return new MyInstrument();
    /// }
    /// @endcode
    //--------------------------------------------------------------------------
    typedef Instrument* tInstrumentConstructor();
}

#endif

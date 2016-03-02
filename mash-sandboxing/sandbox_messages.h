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


/** @file   sandbox_messages.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the messages transmitted between a 'SandboxController' and
    its sandbox
*/

#ifndef _MASH_SANDBOXMESSAGES_H_
#define _MASH_SANDBOXMESSAGES_H_


namespace Mash
{
    enum tSandboxMessage
    {
        SANDBOX_MESSAGE_CREATION_SUCCESSFUL,                            // 0
        SANDBOX_MESSAGE_CREATION_FAILED,

        SANDBOX_MESSAGE_PING,
        SANDBOX_MESSAGE_PONG,
        SANDBOX_MESSAGE_KEEP_ALIVE,

        SANDBOX_MESSAGE_RESPONSE,                                       // 5
        SANDBOX_MESSAGE_UNKNOWN_COMMAND,
        SANDBOX_MESSAGE_ERROR,
        
        SANDBOX_COMMAND_TERMINATE,
        SANDBOX_COMMAND_SET_PLUGINS_FOLDER,
        SANDBOX_COMMAND_LOAD_PLUGIN,                                    // 10
        SANDBOX_COMMAND_USE_MODEL,
        SANDBOX_COMMAND_CREATE_PLUGINS,

        SANDBOX_COMMAND_HEURISTIC_SET_SEED,
        SANDBOX_COMMAND_HEURISTIC_INIT,
        SANDBOX_COMMAND_HEURISTIC_DIM,                                  // 15
        SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_SEQUENCE,
        SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_SEQUENCE,
        SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_IMAGE,
        SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_IMAGE,
        SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_COORDINATES,              // 20
        SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_COORDINATES,
        SANDBOX_COMMAND_HEURISTIC_COMPUTE_SOME_FEATURES,
        SANDBOX_COMMAND_HEURISTIC_TERMINATE,
        SANDBOX_COMMAND_HEURISTIC_REPORT_STATISTICS,

        SANDBOX_COMMAND_LOAD_MODEL,                                     // 25
        SANDBOX_COMMAND_SAVE_MODEL,

        SANDBOX_COMMAND_CLASSIFIER_SET_SEED,
        SANDBOX_COMMAND_CLASSIFIER_SETUP,
        SANDBOX_COMMAND_CLASSIFIER_TRAIN,
        SANDBOX_COMMAND_CLASSIFIER_CLASSIFY,                            // 30
        SANDBOX_COMMAND_CLASSIFIER_REPORT_FEATURES_USED,

        SANDBOX_COMMAND_INPUT_SET_NB_HEURISTICS,
        SANDBOX_COMMAND_INPUT_SET_NB_FEATURES,
        SANDBOX_COMMAND_INPUT_SET_HEURISTIC_NAME,
        SANDBOX_COMMAND_INPUT_SET_HEURISTIC_SEED,                       // 35
        SANDBOX_COMMAND_INPUT_SET_NB_IMAGES,
        SANDBOX_COMMAND_INPUT_SET_NB_LABELS,
        SANDBOX_COMMAND_INPUT_SET_COMPUTE_SOME_FEATURES,
        SANDBOX_COMMAND_INPUT_SET_OBJECTS_IN_IMAGE,
        SANDBOX_COMMAND_INPUT_SET_NEGATIVES_IN_IMAGE,                   // 40
        SANDBOX_COMMAND_INPUT_SET_IMAGE_SIZE,
        SANDBOX_COMMAND_INPUT_SET_IMAGE_IN_TEST_SET,
        SANDBOX_COMMAND_INPUT_SET_ROI_EXTENT,

        SANDBOX_COMMAND_PLANNER_SET_SEED,
        SANDBOX_COMMAND_PLANNER_SETUP,                                  // 45
        SANDBOX_COMMAND_PLANNER_LEARN,
        SANDBOX_COMMAND_PLANNER_CHOOSE_ACTION,
        SANDBOX_COMMAND_PLANNER_REPORT_FEATURES_USED,
        
        SANDBOX_COMMAND_TASK_MODE,
        SANDBOX_COMMAND_TASK_NB_ACTIONS,                                // 50
        SANDBOX_COMMAND_TASK_NB_TRAJECTORIES,
        SANDBOX_COMMAND_TASK_TRAJECTORY_LENGTH,
        SANDBOX_COMMAND_TASK_RESET,
        SANDBOX_COMMAND_TASK_PERFORM_ACTION,
        SANDBOX_COMMAND_TASK_SUGGESTED_ACTION,                          // 55
        
        SANDBOX_COMMAND_PERCEPTION_NB_HEURISTICS,
        SANDBOX_COMMAND_PERCEPTION_NB_FEATURES,
        SANDBOX_COMMAND_PERCEPTION_HEURISTIC_NAME,
        SANDBOX_COMMAND_PERCEPTION_HEURISTIC_SEED,
        SANDBOX_COMMAND_PERCEPTION_NB_VIEWS,                            // 60
        SANDBOX_COMMAND_PERCEPTION_COMPUTE_SOME_FEATURES,
        SANDBOX_COMMAND_PERCEPTION_VIEW_SIZE,
        SANDBOX_COMMAND_PERCEPTION_ROI_EXTENT,

        SANDBOX_COMMAND_INSTRUMENT_SETUP,

        SANDBOX_EVENT_INSTRUMENTS_EXPERIMENT_DONE,                      // 65

        SANDBOX_EVENT_INSTRUMENTS_CLASSIFICATION_EXPERIMENT_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_DONE,
        SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_DONE,                 // 70
        SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_CLASSIFICATION_DONE,
        SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_CLASSIFIER,

        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNING_EXPERIMENT_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_DONE,            // 75
        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_STARTED,
        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_DONE,
        SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_ACTION_CHOOSEN,
        SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_GOALPLANNER,

        SANDBOX_EVENT_INSTRUMENTS_FEATURE_LIST_REPORTED,                // 80
        
        SANDBOX_EVENT_MEMORY_LIMIT_REACHED,
        SANDBOX_EVENT_FORBIDDEN_SYSTEM_CALL,
        
        SANDBOX_MESSAGE_CURRENT_HEURISTIC,
        SANDBOX_MESSAGE_CURRENT_INSTRUMENT,

        SANDBOX_NOTIFICATION_TRAINING_STEP_DONE,                        // 85
    };
}

#endif

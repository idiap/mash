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


/** @file   notifier_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'INotifier' interface
*/

#ifndef _MASH_INOTIFIER_H_
#define _MASH_INOTIFIER_H_

#include <mash-utils/declarations.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface for an object useable by the predictors to send
    ///         notifications about their progress
    ///
    /// The target of the notifications can for instance be the Web platform,
    /// which will display the informations on the appropriate web page.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL INotifier
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        INotifier()
        {
        }

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~INotifier()
        {
        }


        //_____ Notifications __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Inform the target about the progress of the training/learning
        ///         phase
        ///
        /// @param  step            The current step
        /// @param  nbTotalSteps    The total number of steps that the predictor
        ///                         thinks it will perform. If 0, the last
        ///                         provided value is used.
        ///
        /// @remark It is totally acceptable if the value of nbTotalSteps change
        ///         during the course of the training. For instance, the
        ///         predictor might realize that it will need more or less steps
        ///         than what he thought at the beginning.
        ///
        /// @remark Even if nbTotalSteps can be 0 (to use the last provided
        ///         value), it is expected that a value is provided to the very
        ///         first call to onTrainingStepDone(). Also note that if step
        ///         is greater than nbTotalSteps, it will be clamped.
        //----------------------------------------------------------------------
        virtual void onTrainingStepDone(unsigned int step,
                                        unsigned int nbTotalSteps = 0) = 0;
    };
}

#endif

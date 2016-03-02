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


/** @file   notifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Notifier' class
*/

#ifndef _NOTIFIER_H_
#define _NOTIFIER_H_

#include <mash/notifier_interface.h>


// Forward declaration
class Listener;


//--------------------------------------------------------------------------
/// @brief  Concrete implementation of the INotifier interface, that
///         sends the notifications to the client of the Experiment Server
//--------------------------------------------------------------------------
class Notifier: public Mash::INotifier
{
    //_____ Construction / Destruction __________
public:
    //----------------------------------------------------------------------
    /// @brief  Constructor
    //----------------------------------------------------------------------
    Notifier(Listener* pListener);

    //----------------------------------------------------------------------
    /// @brief  Destructor
    //----------------------------------------------------------------------
    virtual ~Notifier();


    //_____ Methods __________
public:
    inline void setReductor(unsigned int value)
    {
        _reductor = value;
    }


    //_____ Notifications __________
public:
    //----------------------------------------------------------------------
    /// @copy   Mash::INotifier::onTrainingStepDone
    //----------------------------------------------------------------------
    virtual void onTrainingStepDone(unsigned int step,
                                    unsigned int nbTotalSteps = 0);


    //_____ Additional notifications __________
public:
    //----------------------------------------------------------------------
    /// @brief  Inform the target about the progress of the phase where the
    ///         train error is computed
    ///
    /// @param  step            The current step
    /// @param  nbTotalSteps    The total number of steps that will be
    ///                         performed. If 0, the last provided value is
    ///                         used.
    //----------------------------------------------------------------------
    void onTrainErrorComputationStepDone(unsigned int step,
                                         unsigned int nbTotalSteps = 0);

    //----------------------------------------------------------------------
    /// @brief  Inform the target about the progress of the test
    ///         phase
    ///
    /// @param  step            The current step
    /// @param  nbTotalSteps    The total number of steps that will be
    ///                         performed. If 0, the last provided value is
    ///                         used.
    //----------------------------------------------------------------------
    void onTestStepDone(unsigned int step, unsigned int nbTotalSteps = 0);


    //_____ Attributes __________
protected:
    Listener*       _pListener;                         ///< Used to send the notifications to the target
    unsigned int    _nbTotalTrainingSteps;              ///< Last value provided to onTrainingStepDone()
    unsigned int    _nbTotalTrainErrorComputationSteps; ///< Last value provided to onTrainErrorComputationStepDone()
    unsigned int    _nbTotalTestSteps;                  ///< Last value provided to onTestStepDone()
    unsigned int    _reductor;                          ///< Used to reduce the number of notifications sent
};

#endif

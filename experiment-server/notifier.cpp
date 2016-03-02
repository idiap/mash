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


/** @file   notifier.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Notifier' class
*/

#include "notifier.h"
#include "listener.h"
#include <memory.h>
#include <stdlib.h>


using namespace std;

using Mash::ArgumentsList;


/************************* CONSTRUCTION / DESTRUCTION *************************/

Notifier::Notifier(Listener* pListener)
: _pListener(pListener), _nbTotalTrainingSteps(0), _nbTotalTestSteps(0),
  _nbTotalTrainErrorComputationSteps(0), _reductor(0)
{
    // Assertions
    assert(pListener);
}


Notifier::~Notifier()
{
}


/****************************** NOTIFICATIONS *********************************/

void Notifier::onTrainingStepDone(unsigned int step, unsigned int nbTotalSteps)
{
    // Assertions
    assert(_pListener);

    if (nbTotalSteps > 0)
        _nbTotalTrainingSteps = nbTotalSteps;

    if (_nbTotalTrainingSteps > 0)
    {
        ArgumentsList args;
        args.add("TRAINING_STEP_DONE");
        args.add((int) min(step, _nbTotalTrainingSteps));
        args.add((int) _nbTotalTrainingSteps);
        
        _pListener->sendResponse("NOTIFICATION", args);
    }
}


/************************ ADDITIONAL NOTIFICATIONS ****************************/

void Notifier::onTrainErrorComputationStepDone(unsigned int step,
                                               unsigned int nbTotalSteps)
{
    // Assertions
    assert(_pListener);

    if (nbTotalSteps > 0)
        _nbTotalTrainErrorComputationSteps = nbTotalSteps;

    if (_nbTotalTrainErrorComputationSteps > 0)
    {
        unsigned int n = min(step, _nbTotalTrainErrorComputationSteps);
        
        if ((_reductor == 0) || (n == _nbTotalTrainErrorComputationSteps) || (n % _reductor == 0))
        {
            ArgumentsList args;
            args.add("TRAIN_ERROR_COMPUTATION_STEP_DONE");
            args.add((int) n);
            args.add((int) _nbTotalTrainErrorComputationSteps);
        
            _pListener->sendResponse("NOTIFICATION", args);
        }
    }
}


void Notifier::onTestStepDone(unsigned int step, unsigned int nbTotalSteps)
{
    // Assertions
    assert(_pListener);

    if (nbTotalSteps > 0)
        _nbTotalTestSteps = nbTotalSteps;

    if (_nbTotalTestSteps > 0)
    {
        unsigned int n = min(step, _nbTotalTestSteps);
        
        if ((_reductor == 0) || (n == _nbTotalTestSteps) || (n % _reductor == 0))
        {
            ArgumentsList args;
            args.add("TEST_STEP_DONE");
            args.add((int) n);
            args.add((int) _nbTotalTestSteps);
        
            _pListener->sendResponse("NOTIFICATION", args);
        }
    }
}

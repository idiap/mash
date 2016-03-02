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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    This goal-planner chooses one action at random
*/

#include <mash-goalplanning/planner.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The 'Random' goal-planner class
//------------------------------------------------------------------------------
class Random: public Planner
{
    //_____ Construction / Destruction __________
public:
    Random();
    virtual ~Random();


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters);
    virtual bool loadModel(PredictorModel &model);
    virtual bool learn(ITask* task);
    virtual unsigned int chooseAction(IPerception* perception);
    virtual bool reportFeaturesUsed(tFeatureList &list);
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:
    unsigned int nbActions;
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
    return new Random();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Random::Random()
: nbActions(0)
{
    srand(time(0));
}


Random::~Random()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Random::setup(const tExperimentParametersList& parameters)
{
    return true;
}


bool Random::loadModel(PredictorModel &model)
{
    return true;
}


bool Random::learn(ITask* task)
{
    // Retrieve the number of actions available
    nbActions = task->nbActions();

    return true;
}


unsigned int Random::chooseAction(IPerception* perception)
{
    // Choose one action at random
    return rand() % nbActions;
}


bool Random::reportFeaturesUsed(tFeatureList &list)
{
    return true;
}


bool Random::saveModel(PredictorModel &model)
{
    return true;
}

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

    This goal-planner is used to benchmark the performance of the system
*/

#include <mash-goalplanning/planner.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The 'Benchmark' goal-planner class
//------------------------------------------------------------------------------
class Benchmark: public Planner
{
    //_____ Construction / Destruction __________
public:
    Benchmark();
    virtual ~Benchmark();


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
    unsigned int nbSteps;
    unsigned int action1;
    unsigned int action2;
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
    return new Benchmark();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Benchmark::Benchmark()
: nbSteps(1000), action1(0), action2(1)
{
}


Benchmark::~Benchmark()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Benchmark::setup(const tExperimentParametersList& parameters)
{
    tExperimentParametersIterator iter;
    
    iter = parameters.find("NB_STEPS");
    if (iter != parameters.end())
    {
        int value = iter->second.getInt(0);
        
        if (value > 0)
            nbSteps = value;
    }

    iter = parameters.find("ACTION1");
    if (iter != parameters.end())
        action1 = iter->second.getInt(0);

    iter = parameters.find("ACTION2");
    if (iter != parameters.end())
        action2 = iter->second.getInt(0);
    
    return true;
}


bool Benchmark::loadModel(PredictorModel &model)
{
    return true;
}


bool Benchmark::learn(ITask* task)
{
    // Cache everything
    unsigned int    nbHeuristics    = task->perception()->nbHeuristics();
    unsigned int    nbMaxFeatures   = 0;
    unsigned int    nbFeaturesTotal = 0;
    unsigned int*   nbFeatures      = new unsigned int[nbHeuristics];
    unsigned int    nbViews         = task->perception()->nbViews();
    coordinates_t*  coordinates     = new coordinates_t[nbViews];
    unsigned int    roiSize         = task->perception()->roiExtent() * 2 + 1;
    unsigned int*   indices         = 0;
    scalar_t*       features        = 0;

    for (unsigned int h = 0; h < nbHeuristics; ++h)
    {
        nbFeatures[h] = task->perception()->nbFeatures(h);
        nbMaxFeatures = max(nbMaxFeatures, nbFeatures[h]);
        nbFeaturesTotal += nbFeatures[h];
    }

    for (unsigned int v = 0; v < nbViews; ++v)
    {
        dim_t size = task->perception()->viewSize(v);
        coordinates[v].x = (size.width - roiSize) >> 1;
        coordinates[v].y = (size.height - roiSize) >> 1;
    }

    indices = new unsigned int[nbMaxFeatures];
    features = new scalar_t[nbMaxFeatures];

    for (unsigned int i = 0; i < nbMaxFeatures; ++i)
        indices[i] = i;


    outStream << "Benchmarking..." << endl;
    
    // Start time monitoring
    struct timeval start, end, duration;
    gettimeofday(&start, 0);


    // Do the job
    for (unsigned int i = 0; i < nbSteps; ++i)
    {
        for (unsigned int v = 0; v < nbViews; ++v)
        {
            for (unsigned int h = 0; h < nbHeuristics; ++h)
            {
                task->perception()->computeSomeFeatures(v, coordinates[v], h,
                                                        nbFeatures[h],
                                                        indices, features);
            }
        }

        scalar_t reward;
        if (i % 2 == 0)
            task->performAction(action1, &reward);
        else
            task->performAction(action2, &reward);
    }


    // End time monitoring
    gettimeofday(&end, 0);
    timersub(&end, &start, &duration);

    float fDuration = ((float) duration.tv_sec) + ((float) duration.tv_usec) * 1e-6f;

    outStream << "Results:" << endl;
    outStream << "   Nb heuristics:           " << nbHeuristics << endl;
    outStream << "   Nb total features:       " << nbFeaturesTotal << endl;
    outStream << "   Nb views:                " << nbViews << endl;
    outStream << "   Nb steps:                " << nbSteps << endl;
    outStream << "   -> Nb features computed: " << nbFeaturesTotal * nbViews * nbSteps << endl;
    outStream << "   -> Duration:             " << fDuration << "s" << endl;
    outStream << "   ->                       " << ((float) nbFeaturesTotal * nbViews * nbSteps) / fDuration << " features/s" << endl;
    outStream << "   ->                       " << ((float) nbSteps) / fDuration << " actions/s" << endl;


    // Cleanup
    delete[] features;
    delete[] indices;
    delete[] coordinates;
    delete[] nbFeatures;

    return true;
}


unsigned int Benchmark::chooseAction(IPerception* perception)
{
    return action1;
}


bool Benchmark::reportFeaturesUsed(tFeatureList &list)
{
    return true;
}


bool Benchmark::saveModel(PredictorModel &model)
{
    return true;
}

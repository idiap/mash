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

    This classifier is used to benchmark the performance of the system
*/

#include <mash-classification/classifier.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The 'AllFeatures' classifier class
//------------------------------------------------------------------------------
class Benchmark: public Classifier
{
    //_____ Construction / Destruction __________
public:
    Benchmark();
    virtual ~Benchmark();


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters);
    virtual bool loadModel(PredictorModel &model, DataReader &internal_data);
    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);
    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results);
    virtual bool reportFeaturesUsed(tFeatureList &list);
    virtual bool saveModel(PredictorModel &model);
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new Benchmark();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Benchmark::Benchmark()
{
}


Benchmark::~Benchmark()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Benchmark::setup(const tExperimentParametersList& parameters)
{
    return true;
}


bool Benchmark::loadModel(PredictorModel &model, DataReader &internal_data)
{
    return true;
}


bool Benchmark::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // Cache everything
    unsigned int    nbHeuristics    = input_set->nbHeuristics();
    unsigned int    nbMaxFeatures   = 0;
    unsigned int    nbFeaturesTotal = 0;
    unsigned int*   nbFeatures      = new unsigned int[nbHeuristics];
    unsigned int    nbImages        = input_set->nbImages();
    unsigned int*   indices         = 0;
    scalar_t*       features        = 0;
    coordinates_t   coordinates;

    for (unsigned int h = 0; h < nbHeuristics; ++h)
    {
        nbFeatures[h] = input_set->nbFeatures(h);
        nbMaxFeatures = max(nbMaxFeatures, nbFeatures[h]);
        nbFeaturesTotal += nbFeatures[h];
    }

    coordinates.x = input_set->roiExtent();
    coordinates.y = coordinates.x;

    indices = new unsigned int[nbMaxFeatures];
    features = new scalar_t[nbMaxFeatures];

    for (unsigned int i = 0; i < nbMaxFeatures; ++i)
        indices[i] = i;


    outStream << "Benchmarking..." << endl;
    
    // Start time monitoring
    struct timeval start, end, duration;
    gettimeofday(&start, 0);


    // Do the job
    for (unsigned int i = 0; i < nbImages; ++i)
    {
        for (unsigned int h = 0; h < nbHeuristics; ++h)
        {
            input_set->computeSomeFeatures(i, coordinates, h, nbFeatures[h],
                                           indices, features);
        }
    }


    // End time monitoring
    gettimeofday(&end, 0);
    timersub(&end, &start, &duration);

    float fDuration = ((float) duration.tv_sec) + ((float) duration.tv_usec) * 1e-6f;

    outStream << "Results:" << endl;
    outStream << "   Nb heuristics:           " << nbHeuristics << endl;
    outStream << "   Nb total features:       " << nbFeaturesTotal << endl;
    outStream << "   Nb images:               " << nbImages << endl;
    outStream << "   -> Nb features computed: " << nbFeaturesTotal * nbImages << endl;
    outStream << "   -> Duration:             " << fDuration << "s" << endl;
    outStream << "   ->                       " << ((float) nbFeaturesTotal * nbImages) / fDuration << " features/s" << endl;
    outStream << "   ->                       " << ((float) nbImages) / fDuration << " images/s" << endl;


    // Cleanup
    delete[] features;
    delete[] indices;
    delete[] nbFeatures;

    return true;
}


bool Benchmark::classify(IClassifierInputSet* input_set, unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results)
{
    results[0] = 1.0f;
    return true;
}


bool Benchmark::reportFeaturesUsed(tFeatureList &list)
{
    return true;
}


bool Benchmark::saveModel(PredictorModel &model)
{
    return true;
}

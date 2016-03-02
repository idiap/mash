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

    For test purposes only!!
    
    A classifier that computes only one feature of the first object of the first
    image (during training) and per coordinates (during classification)
*/

#include <mash-classification/classifier.h>


using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The 'AllFeatures' classifier class
//------------------------------------------------------------------------------
class OneFeature: public Classifier
{
    //_____ Construction / Destruction __________
public:
    OneFeature();
    virtual ~OneFeature();


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

protected:
    scalar_t compute(IClassifierInputSet* input_set, unsigned int image,
                     const coordinates_t& position);
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new OneFeature();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

OneFeature::OneFeature()
{
}


OneFeature::~OneFeature()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool OneFeature::setup(const tExperimentParametersList& parameters)
{
    return true;
}


bool OneFeature::loadModel(PredictorModel &model, DataReader &internal_data)
{
    return true;
}


bool OneFeature::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    unsigned int image = 0;
    
    dim_t imageSize = input_set->imageSize(image);
    
    // Retrieve the objects of the image
    tObjectsList objects;
    input_set->objectsInImage(image, &objects);
    
    // Iterate over all the objects of the image
    tObjectsList::iterator iter, iterEnd;
    unsigned int counter = 0;
    for (iter = objects.begin(), iterEnd = objects.end(); iter != iterEnd; ++iter)
    {
        // If the object doesn't have a correct size (~ the size of the
        // region-of-interest), we don't care about it
        if (!iter->target)
            continue;

        compute(input_set, image, iter->roi_position);
        break;
    }

    return true;
}


bool OneFeature::classify(IClassifierInputSet* input_set, unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results)
{
    scalar_t value = compute(input_set, image, position);

    // Choose one label using the computed value
    unsigned int label = int((value > 1.0f ? value : value * 100)) % input_set->nbLabels();
    
    // Give it a score of 1.0
    results[label] = 1.0f;

    return true;
}


bool OneFeature::reportFeaturesUsed(tFeatureList &list)
{
    // Report some features
    list.push_back(tFeature(0, 0));
    
    return true;
}


scalar_t OneFeature::compute(IClassifierInputSet* input_set, unsigned int image,
                             const coordinates_t& position)
{
    unsigned int heuristic = generator.randomize(input_set->nbHeuristics() - 1);
    unsigned int feature = generator.randomize(input_set->nbFeatures(heuristic) - 1);
    
    scalar_t value = 0.0f;
        
    input_set->computeSomeFeatures(image, position, heuristic, 1, &feature, &value);

    return value;
}


bool OneFeature::saveModel(PredictorModel &model)
{
    return true;
}

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
    
    A classifier that computes every feature once per object (during training)
    and per coordinates (during classification)
*/

#include <mash-classification/classifier.h>


using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The 'AllFeatures' classifier class
//------------------------------------------------------------------------------
class AllFeatures: public Classifier
{
    //_____ Construction / Destruction __________
public:
    AllFeatures();
    virtual ~AllFeatures();


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
    void compute(IClassifierInputSet* input_set, unsigned int image,
                 const coordinates_t& position);
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new AllFeatures();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

AllFeatures::AllFeatures()
{
}


AllFeatures::~AllFeatures()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool AllFeatures::setup(const tExperimentParametersList& parameters)
{
    return true;
}


bool AllFeatures::loadModel(PredictorModel &model, DataReader &internal_data)
{
    return true;
}


bool AllFeatures::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    outStream << "--------------------------------------------------------------------------------" << endl
              << "TRAINING" << endl
              << "Nb heuristics: " << input_set->nbHeuristics() << endl
              << "Nb images:     " << input_set->nbImages() << endl
              << "Nb labels:     " << input_set->nbLabels() << endl;

    // Iterate over all the images
    for (unsigned int image = 0; image < input_set->nbImages(); ++image)
    {
        dim_t imageSize = input_set->imageSize(image);
        
        outStream << "Processing image " << image << "..." << endl
                  << "  Size: " << imageSize.width << "x" << imageSize.height << endl;
        
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

            outStream << "  Object #" << counter << endl
                      << "    Label:    " << iter->label << endl
                      << "    Position: (" << iter->roi_position.x << ", " << iter->roi_position.y << ")" << endl;

            compute(input_set, image, iter->roi_position);
        }
    }

    outStream << endl;

    return true;
}


bool AllFeatures::classify(IClassifierInputSet* input_set, unsigned int image,
                           const coordinates_t& position,
                           tClassificationResults &results)
{
    outStream << "--------------------------------------------------------------------------------" << endl
              << "CLASSIFICATION" << endl
              << "Image:    " << image << endl
              << "Position: (" << position.x << ", " << position.y << ")" << endl
              << endl;

    compute(input_set, image, position);

    outStream << endl;

    // Always returns the same thing
    results[0] = 1.0f;

    return true;
}


bool AllFeatures::reportFeaturesUsed(tFeatureList &list)
{
    return true;
}


bool AllFeatures::saveModel(PredictorModel &model)
{
    return true;
}


void AllFeatures::compute(IClassifierInputSet* input_set, unsigned int image,
                          const coordinates_t& position)
{
    // Iterate over all the heuristics
    for (unsigned int heuristic = 0; heuristic < input_set->nbHeuristics(); ++heuristic)
    {
        unsigned int nbFeatures = input_set->nbFeatures(heuristic);

        outStream << "Heuristic #" << heuristic << "   Nb features: " << nbFeatures << endl;
        
        scalar_t* values = new scalar_t[nbFeatures];
        unsigned int* features = new unsigned int[nbFeatures];
        
        for (unsigned int i = 0; i < nbFeatures; ++i)
            features[i] = i;
        
        if (!input_set->computeSomeFeatures(image, position, heuristic,
                                            nbFeatures, features, values))
        {
            outStream << "ERROR: Failed to compute the features" << endl;
        }
        
        delete[] values;
        delete[] features;
    }
}

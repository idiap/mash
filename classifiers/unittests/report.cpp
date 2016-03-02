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
class Report: public Classifier
{
    //_____ Construction / Destruction __________
public:
    Report();
    virtual ~Report();


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
    return new Report();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Report::Report()
{
}


Report::~Report()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Report::setup(const tExperimentParametersList& parameters)
{
    writer << "setup" << endl;
    
    return true;
}


bool Report::loadModel(PredictorModel &model, DataReader &internal_data)
{
    writer << "loadModel" << endl;
    
    return true;
}


bool Report::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    writer << "train" << endl;

    return true;
}


bool Report::classify(IClassifierInputSet* input_set, unsigned int image,
                      const coordinates_t& position,
                      tClassificationResults &results)
{
    writer << "classify" << endl;

    // Always returns the same thing
    results[0] = 1.0f;

    return true;
}


bool Report::reportFeaturesUsed(tFeatureList &list)
{
    writer << "reportFeaturesUsed" << endl;

    return true;
}


bool Report::saveModel(PredictorModel &model)
{
    writer << "saveModel" << endl;

    return true;
}

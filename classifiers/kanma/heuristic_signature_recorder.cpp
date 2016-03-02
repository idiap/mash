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

    This classifier computes and save the signature of a heuristic in the Data
    Report. The signature is a matrix of computed features over some images.
    
    Thus, it is not a real classifier, and should not be used outside of its
    specific scenario.
    
    It is expected that only one heuristic is available in the Input Set, and
    that all the "signature experiments" are classification ones, using the same
    database, labels and global seed.
*/

#include <mash-classification/classifier.h>
#include <set>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// The classifier class
//------------------------------------------------------------------------------
class HeuristicSignatureRecorder: public Classifier
{
    //_____ Construction / Destruction __________
public:
    HeuristicSignatureRecorder();
    virtual ~HeuristicSignatureRecorder();


    //_____ Methods to implement __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Initialize the classifier
    ///
    /// @param  parameters  The classifier-specific parameters
    /// @return             'true' if successful
    //--------------------------------------------------------------------------
    virtual bool setup(const tExperimentParametersList& parameters);

    //--------------------------------------------------------------------------
    /// @brief  Load a model
    ///
    /// Not used in this scenario
    //--------------------------------------------------------------------------
    virtual bool loadModel(PredictorModel &model, DataReader &internal_data);

    //--------------------------------------------------------------------------
    /// @brief  Train the classifier
    ///
    /// @param      input_set   The Classifier Input Set to use
    /// @param[out] train_error (Optional) The train error. Leave unchanged
    ///                         if you want to let the Framework compute it.
    /// @return                 'true' if successful
    //--------------------------------------------------------------------------
    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);

    //--------------------------------------------------------------------------
    /// @brief  Classify the content of an image at a specific position
    ///
    /// Not used in this scenario
    //--------------------------------------------------------------------------
    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results);

    //--------------------------------------------------------------------------
    /// @brief  Populates the provided list with the features used by the
    ///         classifier
    ///
    /// @retval list    The list of features used
    /// @return         'true' if successful
    ///
    /// @note   This method will only be called after train(), and the 'list' 
    ///         parameter will always be empty.
    //--------------------------------------------------------------------------
    virtual bool reportFeaturesUsed(tFeatureList &list);

    //--------------------------------------------------------------------------
    /// @brief  Save the model trained by the classifier
    ///
    /// Not used in this scenario
    //--------------------------------------------------------------------------
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:
    unsigned int  _nbImages;
    unsigned int  _nbFeatures;
    unsigned int* _features;
};


//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier()
{
    return new HeuristicSignatureRecorder();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

HeuristicSignatureRecorder::HeuristicSignatureRecorder()
: _nbImages(1000), _nbFeatures(1000), _features(0)
{
}


HeuristicSignatureRecorder::~HeuristicSignatureRecorder()
{
    delete[] _features;
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool HeuristicSignatureRecorder::setup(const tExperimentParametersList& parameters)
{
    // Declarations
    tExperimentParametersIterator iter;
    
    // Retrieve the maximum number of images to use
    iter = parameters.find("NB_IMAGES");
    if (iter != parameters.end())
        _nbImages = iter->second.getInt(0);

    // Retrieve the maximum number of features to use
    iter = parameters.find("NB_MAX_FEATURES");
    if (iter != parameters.end())
        _nbFeatures = iter->second.getInt(0);

    return true;
}


bool HeuristicSignatureRecorder::loadModel(PredictorModel &model, DataReader &internal_data)
{
    // Not used
    return true;
}


bool HeuristicSignatureRecorder::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // Check that there is only one heuristic in the Input Set
    if (input_set->nbHeuristics() != 1)
    {
        outStream << "This classifier can only be used with one heuristic" << endl;
        return false;
    }

    // Check that the experiment is a classification one
    if (input_set->isDoingDetection())
    {
        outStream << "This classifier can't be used in Object Detection experiments" << endl;
        return false;
    }

    // Check that there is enough features and images
    _nbImages = min(_nbImages, input_set->nbImages());
    _nbFeatures = min(_nbFeatures, input_set->nbFeatures(0));

    outStream << "Parameters:" << endl
              << "    - Nb images:   " << _nbImages << endl
              << "    - Nb features: " << _nbFeatures << endl;


    // Selection of the images
    outStream << "Selection of the images..." << endl;

    typedef std::set<unsigned int> tImageList;

    tImageList* all_images = new tImageList[input_set->nbLabels()];
    tImageList selected_images;
    
    for (unsigned int i = 0; i < input_set->nbImages(); ++i)
    {
        tObjectsList objects;
        input_set->objectsInImage(i, &objects);
        
        all_images[objects[0].label].insert(i);
    }

    unsigned int nbImagesPerLabelMean = _nbImages / input_set->nbLabels();
    
    for (unsigned int i = 0; i < input_set->nbLabels(); ++i)
    {
        unsigned int nb = min((unsigned int) all_images[i].size(), nbImagesPerLabelMean);
        unsigned int start = selected_images.size();
        
        while (selected_images.size() < start + nb)
        {
            unsigned int index = generator.randomize((unsigned int) all_images[i].size() - 1);
            
            tImageList::iterator iter = all_images[i].begin();
            for (unsigned int k = 0; k < index; ++k, ++iter) {}

            unsigned int image = *iter;

            selected_images.insert(image);
            all_images[i].erase(image);
        }
    }

    while (selected_images.size() < _nbImages)
    {
        for (unsigned int i = 0; (i < input_set->nbLabels()) && (selected_images.size() < _nbImages); ++i)
        {
            if (!all_images[i].empty())
            {
                unsigned int index = generator.randomize((unsigned int) all_images[i].size() - 1);

                tImageList::iterator iter = all_images[i].begin();
                for (unsigned int k = 0; k < index; ++k, ++iter) {}

                unsigned int image = *iter;

                if (selected_images.find(image) == selected_images.end())
                {
                    selected_images.insert(image);
                    all_images[i].erase(image);
                }
            }
        }
    }

    delete[] all_images;


    // Selection of the features
    outStream << "Selection of the features..." << endl;

    typedef std::set<unsigned int> tFeaturesList;
    tFeaturesList all_features;

    _features = new unsigned int[_nbFeatures];

    for (unsigned int f = 0; f < input_set->nbFeatures(0); ++f)
        all_features.insert(f);
    
    for (unsigned int f = 0; f < _nbFeatures; ++f)
    {
        unsigned int index = generator.randomize((unsigned int) all_features.size() - 1);
        
        tFeaturesList::iterator iter = all_features.begin();
        for (unsigned int k = 0; k < index; ++k, ++iter) {}

        _features[f] = *iter;

        all_features.erase(*iter);
    }
    
    
    // Computation of the signature
    outStream << "Computation of the signature..." << endl;
    
    scalar_t* signature = new scalar_t[_nbFeatures * _nbImages];
    scalar_t* values = new scalar_t[_nbFeatures];

    scalar_t* pColumn = signature;
    
    tImageList::iterator iter, iterEnd;
    for (iter = selected_images.begin(), iterEnd = selected_images.end();
         iter != iterEnd; ++iter)
    {
        // Retrieve the objects in the image
        tObjectsList objects;
        unsigned int image = *iter;
        input_set->objectsInImage(image, &objects);

        if (!input_set->computeSomeFeatures(image, objects[0].roi_position, 0,
                                            _nbFeatures, _features, values))
        {
            outStream << "Failed to compute the features on image #" << image << endl;

            delete[] signature;
            delete[] values;

            return false;
        }

        // Copy them in the signature matrix
        scalar_t* pDst = pColumn;
        for (unsigned int f = 0; f < _nbFeatures; ++f)
        {
            *pDst = values[f];
            pDst += _nbImages;
        }
        
        ++pColumn;
    }


    // Saving of the signature
    outStream << "Saving of the signature..." << endl;
    
    writer << "FORMAT 1.0" << endl
           << "HEURISTIC " << input_set->heuristicName(0) << endl
           << "MATRIX_SIZE " << _nbImages << "x" << _nbFeatures << endl;
    
    for (iter = selected_images.begin(), iterEnd = selected_images.end();
        iter != iterEnd; ++iter)
    {
        // Retrieve the objects in the image
        tObjectsList objects;
        input_set->objectsInImage(*iter, &objects);

        uint32_t label = objects[0].label;

        writer.write((int8_t*) &label, sizeof(uint32_t));
    }

    writer.write((int8_t*) signature, _nbFeatures * _nbImages * sizeof(scalar_t));


    // Cleanup
    delete[] signature;
    delete[] values;

    train_error = 1.0f;

    return true;
}


bool HeuristicSignatureRecorder::classify(IClassifierInputSet* input_set,
                                          unsigned int image,
                                          const coordinates_t& position,
                                          tClassificationResults &results)
{
    // Not used
    results[0] = 1.0f;
    return true;
}


bool HeuristicSignatureRecorder::reportFeaturesUsed(tFeatureList &list)
{
    for (unsigned int i = 0; i < _nbFeatures; ++i)
        list.push_back(tFeature(0, _features[i]));
    
    return true;
}


bool HeuristicSignatureRecorder::saveModel(PredictorModel &model)
{
    // Not used
    return true;
}

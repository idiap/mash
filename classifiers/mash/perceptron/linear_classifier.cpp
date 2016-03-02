#include "linear_classifier.h"
#include <mash-utils/random_number_generator.h>
#include <memory.h>
#include <algorithm>
#include <set>


using namespace Mash;
using namespace std;


/****************************** STATIC ATTRIBUTES *****************************/

unsigned int LinearClassifier::nbMaxFeaturesPerHeuristic    = 100;
unsigned int LinearClassifier::nbMaxTrainingSteps           = 200;
unsigned int LinearClassifier::nbMaxAdaptationSteps         = 20;
unsigned int LinearClassifier::maxCacheSize                 = 1024 * 1024 * 1024;


/************************* CONSTRUCTION / DESTRUCTION *************************/

LinearClassifier::LinearClassifier()
: _label(0), _model(0), _modelSize(0)
{
}


LinearClassifier::~LinearClassifier()
{
    for (unsigned int i = 0; i < _modelSize; ++i)
    {
        delete[] _model[i].features;
        delete[] _model[i].weights;
    }

    delete[] _model;
}


/********************************** METHODS ***********************************/

void LinearClassifier::setup(unsigned int label, const Mash::Notifier& notifier,
                             const Mash::OutStream& outStream)
{
    _label      = label;
    _notifier   = notifier;
    _outStream  = outStream;
}


bool LinearClassifier::train(IClassifierInputSet* input_set,
                             unsigned int seed)
{
    // Check that the linear classifier isn't already trained
    if (_model)
        return false;
    
    _outStream << "Training linear classifier for label " << _label << endl;
    
    _notifier.onTrainingStepDone(nbMaxTrainingSteps * _label);
    
    // Create a new model big enough to hold all the heuristics
    _modelSize = input_set->nbHeuristics();
    _model = new tHeuristicEntry[_modelSize];

    // Selection of the features at random
    RandomNumberGenerator generator;
    generator.setSeed(seed);

    typedef std::set<unsigned int> tFeaturesList;

    unsigned int nbFeaturesTotal = 0;

    for (unsigned int h = 0; h < _modelSize; ++h)
    {
        _model[h].heuristic   = h;
        _model[h].nbFeatures = min(nbMaxFeaturesPerHeuristic, input_set->nbFeatures(h));
        _model[h].features    = new unsigned int[_model[h].nbFeatures];
        _model[h].weights     = new scalar_t[_model[h].nbFeatures];

        nbFeaturesTotal += _model[h].nbFeatures;

        tFeaturesList all_features;

        for (unsigned int f = 0; f < input_set->nbFeatures(h); ++f)
            all_features.insert(f);

        for (unsigned int f = 0; f < _model[h].nbFeatures; ++f)
        {
            unsigned int index = generator.randomize((unsigned int) all_features.size() - 1);

            tFeaturesList::iterator iter = all_features.begin();
            for (unsigned int k = 0; k < index; ++k, ++iter) {}

            _model[h].features[f] = *iter;
            _model[h].weights[f] = 1.0f;

            all_features.erase(*iter);
        }
    }
    
    _outStream << "    " << nbFeaturesTotal << " features selected in "
               << _modelSize << " heuristics" << endl;
    
    // Actual training of the model
    return learn(input_set, nbFeaturesTotal, nbMaxTrainingSteps);
}


bool LinearClassifier::adapt(IClassifierInputSet* input_set,
                             unsigned int seed)
{
    // Check that the linear classifier is already trained
    if (!_model)
        return false;
    
    _outStream << "Adapting linear classifier for label " << _label << endl;
    
    _notifier.onTrainingStepDone(nbMaxAdaptationSteps * _label);
    
    // Create a new model big enough to hold all the heuristics
    unsigned int previousModelSize = _modelSize;
    tHeuristicEntry* previousModel = _model;

    _modelSize = input_set->nbHeuristics();
    _model = new tHeuristicEntry[_modelSize];

    // Copy the previous model into the new one
    memcpy(_model, previousModel, previousModelSize * sizeof(tHeuristicEntry));

    // Retrieve the number of features already in the model
    unsigned int nbFeaturesTotal = 0;
    for (unsigned int h = 0; h < previousModelSize; ++h)
        nbFeaturesTotal += _model[h].nbFeatures;

    // Select the features of the heuristics not already in the model at random
    RandomNumberGenerator generator;
    generator.setSeed(seed);

    unsigned int currentHeuristicIndex = 0;
    for (unsigned int h = previousModelSize; h < _modelSize; ++h)
    {
        // Search the next heuristic not already in the model
        while (input_set->isHeuristicUsedByModel(currentHeuristicIndex))
            ++currentHeuristicIndex;
        
        _model[h].heuristic   = currentHeuristicIndex;
        _model[h].nbFeatures  = min(nbMaxFeaturesPerHeuristic, input_set->nbFeatures(currentHeuristicIndex));
        _model[h].features    = new unsigned int[_model[h].nbFeatures];
        _model[h].weights     = new scalar_t[_model[h].nbFeatures];

        nbFeaturesTotal += _model[h].nbFeatures;
        
        // Selection of the features
        typedef std::set<unsigned int> tFeaturesList;
        tFeaturesList all_features;

        for (unsigned int f = 0; f < input_set->nbFeatures(currentHeuristicIndex); ++f)
            all_features.insert(f);

        for (unsigned int f = 0; f < _model[h].nbFeatures; ++f)
        {
            unsigned int index = generator.randomize((unsigned int) all_features.size() - 1);

            tFeaturesList::iterator iter = all_features.begin();
            for (unsigned int k = 0; k < index; ++k, ++iter) {}

            _model[h].features[f] = *iter;
            _model[h].weights[f] = 1.0f;

            all_features.erase(*iter);
        }
    }
    
    _outStream << "    " << nbFeaturesTotal << " features selected in "
               << _modelSize << " heuristics" << endl;
    
    // Actual adaptation of the model
    return learn(input_set, nbFeaturesTotal, nbMaxAdaptationSteps);
}


bool LinearClassifier::learn(Mash::IClassifierInputSet* input_set,
                             unsigned int nbFeaturesTotal, unsigned int nbMaxSteps)
{
    // Allocate memory structures used to cache data to speed-up the computation
    uint64_t cache_size = min(uint64_t(maxCacheSize) / sizeof(scalar_t),
                              uint64_t(nbFeaturesTotal) * input_set->nbImages());

    cache_size -= cache_size % nbFeaturesTotal;

    scalar_t* cached_feature_values = new scalar_t[cache_size];
    scalar_t* feature_values = new scalar_t[nbMaxFeaturesPerHeuristic];
    scalar_t* scores = new scalar_t[input_set->nbImages()];
    int* labels = new int[input_set->nbImages()];

    // For each learning step
    for (unsigned int step = 0; step < nbMaxSteps; ++step)
    {
        // Iterate over all the images
        for (unsigned int image = 0; image < input_set->nbImages(); ++image)
        {
            // Retrieve the objects
            tObjectsList objects;
            input_set->objectsInImage(image, &objects);
            
            // Iterate over all the objects of the image and compute the response of
            // the linear classifier
            // Note: In image classification, there is only one object per image
            tObjectsList::iterator iter, iterEnd;
            for (iter = objects.begin(), iterEnd = objects.end(); iter != iterEnd; ++iter)
            {
                // If the object doesn't have a correct size (~ the size of the
                // region-of-interest), we don't care about it
                if (!iter->target)
                    continue;
                    
                // Iterate over all the heuristics in the model to compute the features
                scalar_t* values = 0;
                scores[image] = 0.0f;
                
                if ((image + 1) * nbFeaturesTotal <= cache_size)
                    values = &cached_feature_values[image * nbFeaturesTotal];
                else
                    values = feature_values;

                for (unsigned int heuristic = 0; heuristic < _modelSize; ++heuristic)
                {
                    // Only compute the features once, used the cached ones later
                    if ((step == 0) || (values == feature_values))
                    {
                        if (!input_set->computeSomeFeatures(image, iter->roi_position,
                                                            _model[heuristic].heuristic,
                                                            _model[heuristic].nbFeatures,
                                                            _model[heuristic].features,
                                                            values))
                        {
                            delete[] cached_feature_values;
                            delete[] feature_values;
                            delete[] scores;
                            delete[] labels;
                            return false;
                        }
                        
                        labels[image] = (iter->label == _label ? 1 : -1);
                    }

                    // Compute the response of the linear classifier on the current image
                    for (unsigned int f = 0; f < _model[heuristic].nbFeatures; ++f)
                        scores[image] += _model[heuristic].weights[f] * values[f];

                    if (values != feature_values)
                        values += _model[heuristic].nbFeatures;
                }
            }
        }

        // Update the linear classifier if necessary (perceptron rule)
        unsigned int nbErrors = 0;

        for (unsigned int image = 0; image < input_set->nbImages(); ++image)
        {
            if (scores[image] * labels[image] < 0.0f)
            {
                // The features are in the cache
                if ((image + 1) * nbFeaturesTotal <= cache_size)
                {
                    scalar_t* values = &cached_feature_values[image * nbFeaturesTotal];

                    for (unsigned int heuristic = 0; heuristic < _modelSize; ++heuristic)
                    {
                        for (unsigned int f = 0; f < _model[heuristic].nbFeatures; ++f)
                            _model[heuristic].weights[f] += labels[image] * values[f];

                        values += _model[heuristic].nbFeatures;
                    }
                }
                
                // The features aren't in the cache
                else
                {
                    scalar_t* values = feature_values;

                    tObjectsList objects;
                    input_set->objectsInImage(image, &objects);

                    tObjectsList::iterator iter, iterEnd;
                    for (iter = objects.begin(), iterEnd = objects.end(); iter != iterEnd; ++iter)
                    {
                        if (!iter->target)
                            continue;

                        for (unsigned int heuristic = 0; heuristic < _modelSize; ++heuristic)
                        {
                            if (!input_set->computeSomeFeatures(image, iter->roi_position,
                                                                _model[heuristic].heuristic,
                                                                _model[heuristic].nbFeatures,
                                                                _model[heuristic].features,
                                                                values))
                            {
                                delete[] cached_feature_values;
                                delete[] feature_values;
                                delete[] scores;
                                delete[] labels;
                                return false;
                            }

                            for (unsigned int f = 0; f < _model[heuristic].nbFeatures; ++f)
                                _model[heuristic].weights[f] += labels[image] * values[f];
                        }
                    }
                }

                ++nbErrors;
            }
        }

        _outStream << "    Step #" << step << ", nb_errors = " << nbErrors << endl;

        // Send a notification about our progress
        _notifier.onTrainingStepDone(nbMaxSteps * _label + step);

        // 
        if (nbErrors == 0)
            break;
    }
    
    // Cleanup
    delete[] cached_feature_values;
    delete[] feature_values;
    delete[] scores;
    delete[] labels;

    // Send a notification about our progress
    _notifier.onTrainingStepDone(nbMaxSteps * (_label + 1));
    
    return true;
}


bool LinearClassifier::classify(IClassifierInputSet* input_set,
                                unsigned int image,
                                const coordinates_t& position,
                                scalar_t &score)
{
    scalar_t* feature_values = new scalar_t[nbMaxFeaturesPerHeuristic];

    // Classify the image position
    score = 0.0f;
    for (unsigned int heuristic = 0; heuristic < _modelSize; ++heuristic)
    {
        if (!input_set->computeSomeFeatures(image, position,
                                            _model[heuristic].heuristic,
                                            _model[heuristic].nbFeatures,
                                            _model[heuristic].features,
                                            feature_values))
        {
            delete[] feature_values;
            return false;
        }

        for (unsigned int f = 0; f < _model[heuristic].nbFeatures; ++f)
            score += _model[heuristic].weights[f] * feature_values[f];
    }
    
    delete[] feature_values;

    return true;
}


void LinearClassifier::reportFeaturesUsed(tFeatureList &list)
{
    for (unsigned int i = 0; i < _modelSize; ++i)
    {
        for (unsigned int j = 0; j < _model[i].nbFeatures; ++j)
        {
            list.push_back(tFeature(_model[i].heuristic, _model[i].features[j]));
        }
    }
}

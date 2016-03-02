/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Francois Fleuret (francois.fleuret@idiap.ch)
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


/** Author: Francois Fleuret (francois.fleuret@idiap.ch)
 *
 * This is a very simple Adaboost-like classifier.
 *
 * (1) It creates a two-class classifier for each of the labels, and
 * trains it in one vs. all. Each one is composed of a total of
 * NB_STUMPS stumps.
 *
 * (2) It samples NB_FEATURES_SAMPLED_PER_HEURISTIC features every
 * FEATURE_SAMPLING_PERIOD boosting steps from each heuristic, and at
 * every iteration, adds in each one of the two-class classifiers a
 * stump based on one of these features. If FEATURE_SAMPLING_PERIOD
 * is zero, then the features are sampled only once, when the process
 * starts.
 *
 * (3) Each stump has two weights: One for the 'positive' response,
 * and one for the 'negative'.
 *
 * Default:
 *
 * NB_STUMPS = 100
 * NB_FEATURES_SAMPLED_PER_HEURISTIC = 100
 * FEATURE_SAMPLING_PERIOD = 0
 *
 */

#include <MLInputSet/MashInputSet.h>
#include <mash-classification/classifier.h>

#include <cmath>

using namespace Mash;
using namespace std;

#include "misc.h"
#include "fusion_sort.h"

//////////////////////////////////////////////////////////////////////

inline scalar_t individual_loss(scalar_t label, scalar_t response) {
    return exp( - label * response);
}

inline scalar_t individual_loss_derivative(scalar_t label, scalar_t response) {
    return - label * exp( - label * response);
}

class SimpleAdaboost: public Classifier {
public:
    SimpleAdaboost();
    virtual ~SimpleAdaboost();
    
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
    unsigned int _nb_labels;
    unsigned int _nb_stumps_per_label;
    unsigned int _nb_features_sampled_per_heuristic;
    unsigned int _feature_sampling_period;
    unsigned int _nb_samples_sampled_per_round;
    
    unsigned int *_nb_features_from_heuristic;
    scalar_t *_stump_thresholds;
    scalar_t *_stump_weights0;
    scalar_t *_stump_weights1;
    unsigned int *_stump_heuristics;
    unsigned int *_stump_features;
    unsigned int *_stump_labels;
    
    void free();
    void allocate(int nb_heuristics);
    
    unsigned int nb_samples_in_input_set(IClassifierInputSet *input_set);
    
    void collect_labels_from_input_set(IClassifierInputSet *input_set,
            unsigned int *labels);
    
    scalar_t robust_sampling(int nb, scalar_t *weights, int nb_to_sample, unsigned int *sampled);
    
    void compute_feature_responses(IClassifierInputSet *input_set,
            unsigned int nb_features_sampled_per_heuristic,
            unsigned int *feature_index_blocks,
            scalar_t **result_feature_responses,
            unsigned int *sample_indexes,
            unsigned int nb_samples);
    
    scalar_t loss(unsigned int nb_samples, unsigned int nb_labels,
            unsigned int *sample_labels, scalar_t **responses);
    
    unsigned int nb_errors(unsigned int nb_samples, unsigned int nb_labels,
            unsigned int *sample_labels, scalar_t **responses);
    
    scalar_t compute_bounded_weight(scalar_t num, scalar_t den);
    
    void compute_stump_weights(unsigned int nb_samples, unsigned int *sample_labels,
            unsigned int positive_label,
            unsigned int stump_feature, scalar_t stump_threshold,
            scalar_t *classifier_responses, scalar_t *feature_responses,
            scalar_t *result_weight0, scalar_t *result_weight1);
    
    void compute_best_feature_responses(IClassifierInputSet *input_set,
            scalar_t *result_feature_responses,
            unsigned int *best_feature,
            unsigned int h, unsigned int nb_samples);
    void pick_best_stump(unsigned int nb_samples,
            unsigned int nb_features,
            scalar_t *responses, scalar_t *edges,
            scalar_t **feature_responses,
            unsigned int *result_feature, scalar_t *result_threshold,
            unsigned int *sample_indexes);
    
    void reorganize_stumps_by_blocks(int nb_heuristics);
};

extern "C" Classifier* new_classifier() {
    return new SimpleAdaboost();
}

/************************* CONSTRUCTION / DESTRUCTION *************************/

SimpleAdaboost::SimpleAdaboost() {
    _nb_features_from_heuristic = 0;
    _feature_sampling_period = 0;
    
    _stump_thresholds = 0;
    _stump_weights0 = 0;
    _stump_weights1 = 0;
    
    _stump_heuristics = 0;
    _stump_features = 0;
    _stump_labels = 0;
}

SimpleAdaboost::~SimpleAdaboost() {
    free();
}

void SimpleAdaboost::free() {
    delete[] _nb_features_from_heuristic;
    
    delete[] _stump_thresholds;
    delete[] _stump_weights0;
    delete[] _stump_weights1;
    
    delete[] _stump_heuristics;
    delete[] _stump_features;
    delete[] _stump_labels;
}

void SimpleAdaboost::allocate(int nb_heuristics) {
    _nb_features_from_heuristic = new unsigned int[nb_heuristics];
    
    _stump_thresholds = new scalar_t[_nb_stumps_per_label * _nb_labels];
    _stump_weights0 = new scalar_t[_nb_stumps_per_label * _nb_labels];
    _stump_weights1 = new scalar_t[_nb_stumps_per_label * _nb_labels];
    
    _stump_heuristics = new unsigned int[_nb_stumps_per_label * _nb_labels];
    _stump_features =  new unsigned int[_nb_stumps_per_label * _nb_labels];
    _stump_labels =  new unsigned int[_nb_stumps_per_label * _nb_labels];
}

/************************* IMPLEMENTATION OF Classifier ***********************/

bool SimpleAdaboost::setup(const tExperimentParametersList& parameters) {
    _nb_stumps_per_label = 100;
    _nb_features_sampled_per_heuristic = 100;
    _nb_samples_sampled_per_round = 3000;
    
    _feature_sampling_period = 0;
    _nb_labels = 0;
    
    for(tExperimentParametersIterator iter = parameters.begin();
    iter != parameters.end(); ++iter) {
        if(iter->first == "NB_STUMPS") {
            _nb_stumps_per_label = iter->second.getInt(0);
        }
        else if(iter->first == "NB_FEATURES_SAMPLED_PER_HEURISTIC") {
            _nb_features_sampled_per_heuristic = iter->second.getInt(0);
        }
	else if(iter->first == "NB_SAMPLES_SAMPLED_PER_HEURISTIC") {
            _nb_samples_sampled_per_round = iter->second.getInt(0);
        }
        else if(iter->first == "FEATURE_SAMPLING_PERIOD") {
            _feature_sampling_period = iter->second.getInt(0);
        }
        else {
            outStream << "Warning: unknown parameter '" << iter->first << "'."
                    << std::endl;
        }
    }
    
    outStream << "SETUP "
            << "_nb_features_sampled_per_heuristic "
            << _nb_features_sampled_per_heuristic
            << " _nb_stumps_per_label "
            << _nb_stumps_per_label
            << " _feature_sampling_period "
            << _feature_sampling_period
            << endl;
    
    return true;
}

bool SimpleAdaboost::loadModel(PredictorModel &model, DataReader &internal_data) {
    // TODO: Implement it
    return true;
}

scalar_t SimpleAdaboost::compute_bounded_weight(scalar_t num, scalar_t den) {
    scalar_t result;
    const scalar_t max_weight = 10;
    
    if(num > 0 && den > 0) {
        result = 0.5 * log(num / den);
        if(result < -max_weight)
            result = -max_weight;
        else if(result > max_weight)
            result = max_weight;
    } else {
        if(num > 0)
            return   max_weight;
        else if(den > 0)
            return - max_weight;
        else
            return 0.0;
    }
    
    return result;
}

void SimpleAdaboost::pick_best_stump(unsigned int nb_samples,
        unsigned int nb_features,
        scalar_t *responses, scalar_t *edges,
        scalar_t **feature_responses,
        unsigned int *result_feature,
        scalar_t *result_threshold,
        unsigned int *sample_indexes) {
    
    int *indexes = new int[nb_samples];
    int *sorted_indexes = new int[nb_samples];
    scalar_t *one_feature_responses = new scalar_t[nb_samples];
    int initialized = 0;
    
    scalar_t max_loss_derivative = 0;
    *result_feature = -1;
    
    for(int f = 0; f < nb_features; f++) {
        scalar_t s = 0;
        
        for(int n = 0; n < nb_samples; n++) {
            one_feature_responses[n] = feature_responses[n][f];
            indexes[n] = n;
            s += edges[sample_indexes[n]];
        }
        
        indexed_fusion_sort(nb_samples, indexes, sorted_indexes, one_feature_responses);
        
        for(int n = 0; n < nb_samples - 1; n++) {
            int i = sorted_indexes[n];
            int j = sorted_indexes[n + 1];
            s -= 2 * edges[sample_indexes[i]];
            if(one_feature_responses[j] > one_feature_responses[i]) {
                if(abs(s) > abs(max_loss_derivative)) {
                    max_loss_derivative = s;
                    *result_feature = f;
                    *result_threshold = (one_feature_responses[i] +
                            one_feature_responses[j]) / 2;
                    initialized = 1;
                }
            }
        }
        
        if(f == 0 || !initialized) {
            // Note that this value of max_loss_derivative is not correct,
            // but we do not care, anything better will be used in the next
            // loops.
            max_loss_derivative = 0;
            *result_feature = f;
            *result_threshold = one_feature_responses[0];
            initialized = 1;
        }
    }
    
    ASSERT(initialized);
    
    delete[] one_feature_responses;
    delete[] indexes;
    delete[] sorted_indexes;
}

void SimpleAdaboost::reorganize_stumps_by_blocks(int nb_heuristics) {
    int n = 0, pred_n = 0;
    
    for(unsigned int h = 0; h < nb_heuristics; h++) {
        for(unsigned int q = 0; q < _nb_labels * _nb_stumps_per_label; q++) {
            if(_stump_heuristics[q] == h) {
                if(q != n) {
                    exchange(_stump_thresholds[q], _stump_thresholds[n]);
                    exchange(_stump_weights0[q], _stump_weights0[n]);
                    exchange(_stump_weights1[q], _stump_weights1[n]);
                    exchange(_stump_heuristics[q], _stump_heuristics[n]);
                    exchange(_stump_features[q], _stump_features[n]);
                    exchange(_stump_labels[q], _stump_labels[n]);
                }
                n++;
            }
        }
        
        _nb_features_from_heuristic[h] = n - pred_n;
        
        pred_n = n;
    }
}

void SimpleAdaboost::compute_feature_responses(IClassifierInputSet *input_set,
        unsigned int nb_features_per_heuristic,
        unsigned int *feature_index_blocks,
        scalar_t **result_feature_responses,
        unsigned int  *sample_indexes,
        unsigned int nb_samples) {
    
    outStream << "COMPUTING_FEATURE_RESPONSES" << endl;
    
    for(unsigned int i = 0; i < nb_samples; i++) {
        tObjectsList objects;
        input_set->objectsInImage(sample_indexes[i], &objects);
        for(unsigned int h = 0; h < input_set->nbHeuristics(); h++) {
            
            input_set->computeSomeFeatures(sample_indexes[i],
                    objects[0].roi_position,
                    h,
                    nb_features_per_heuristic,
                    feature_index_blocks + h * nb_features_per_heuristic,
                    result_feature_responses[i] + h * nb_features_per_heuristic);
            
        }
    }
}

void SimpleAdaboost::compute_best_feature_responses(IClassifierInputSet *input_set,
        scalar_t *result_feature_responses,
        unsigned int *best_feature,
        unsigned int h, unsigned int nb_samples) {
    
    outStream << "COMPUTING_BEST_FEATURE_RESPONSES" << endl;
    
    for(unsigned int i = 0; i < nb_samples; i++) {
        tObjectsList objects;
        input_set->objectsInImage(i, &objects);
        scalar_t foo[1];
        input_set->computeSomeFeatures(i,
                objects[0].roi_position,
                h,
                1,
                best_feature,
                foo);
        result_feature_responses[i] = foo[0];
        
    }
}

scalar_t SimpleAdaboost::loss(unsigned int nb_samples, unsigned int nb_labels,
        unsigned int *sample_labels, scalar_t **responses) {
    scalar_t s = 0;
    
    for(int n = 0; n < nb_samples; n++) {
        for(int l = 0; l < nb_labels; l++) {
            if(sample_labels[n] == l) {
                s += individual_loss( 1.0, responses[l][n]);
            } else {
                s += individual_loss(-1.0, responses[l][n]);
            }
        }
    }
    
    return s;
}

unsigned int SimpleAdaboost::nb_errors(unsigned int nb_samples, unsigned int nb_labels,
        unsigned int *sample_labels, scalar_t **responses) {
    unsigned int ne = 0;
    
    for(int n = 0; n < nb_samples; n++) {
        int best_l = 0;
        for(int l = 1; l < nb_labels; l++) {
            if(responses[l][n] > responses[best_l][n]) best_l = l;
        }
        if(sample_labels[n] != best_l) {
            ne++;
        }
    }
    
    return ne;
}

void SimpleAdaboost::compute_stump_weights(unsigned int nb_samples, unsigned int *sample_labels,
        unsigned int positive_label,
        unsigned int stump_feature, scalar_t stump_threshold,
        scalar_t *classifier_responses, scalar_t *feature_responses,
        scalar_t *result_weight0, scalar_t *result_weight1) {
    
    scalar_t num0 = 0, den0 = 0, num1 = 0, den1 = 0;
    
    for(int n = 0; n < nb_samples; n++) {
        if(feature_responses[n] >= stump_threshold) {
            if(sample_labels[n] == positive_label) {
                num1 += individual_loss(   1.0, classifier_responses[n] );
            } else {
                den1 += individual_loss( - 1.0, classifier_responses[n] );
            }
        } else {
            if(sample_labels[n] == positive_label) {
                num0 += individual_loss(   1.0, classifier_responses[n] );
            } else {
                den0 += individual_loss( - 1.0, classifier_responses[n] );
            }
        }
    }
    
    *result_weight0 = compute_bounded_weight(num0, den0);
    *result_weight1 = compute_bounded_weight(num1, den1);
}

unsigned int SimpleAdaboost::nb_samples_in_input_set(IClassifierInputSet *input_set) {
    return input_set->nbImages();
}

void SimpleAdaboost::collect_labels_from_input_set(IClassifierInputSet *input_set,
        unsigned int *labels) {
    
    int s = 0;
    
    for(unsigned int i = 0; i < input_set->nbImages(); i++) {
        tObjectsList objects;
        input_set->objectsInImage(i, &objects);
        labels[s++] = objects[0].label;
    }
}

bool SimpleAdaboost::train(IClassifierInputSet* input_set, scalar_t &train_error) {
    int nb_samples;
    int total_nb_features;
    
    scalar_t **feature_responses;
    scalar_t *best_feature_responses;
    
    unsigned int *heuristic_indexes;
    unsigned int *feature_indexes;
    
    scalar_t **classifiers_responses;
    scalar_t *edges;
    unsigned int *sample_labels;
    scalar_t *my_sample_weights;
    unsigned int *sample_indexes;
    
    free();
    
    _nb_labels = input_set->nbLabels();
    
    allocate(input_set->nbHeuristics());
    
    // Compute the total number of samples
    
    nb_samples = nb_samples_in_input_set(input_set);
    
    outStream << "TRAINING nb_samples "
            << nb_samples
            << " _nb_labels "
            << _nb_labels
            << endl;
    
    total_nb_features = _nb_features_sampled_per_heuristic * input_set->nbHeuristics();
    feature_responses = allocate_array<scalar_t>(_nb_samples_sampled_per_round, total_nb_features);
    best_feature_responses = new scalar_t[nb_samples];
    my_sample_weights =   new scalar_t[nb_samples];
    heuristic_indexes = new unsigned int[total_nb_features];
    feature_indexes = new unsigned int[total_nb_features];
    
    classifiers_responses = allocate_clear_array<scalar_t>(_nb_labels, nb_samples);
    edges = new scalar_t[nb_samples];
    sample_labels = new unsigned int[nb_samples];
    sample_indexes = new unsigned int[_nb_samples_sampled_per_round];
    
    collect_labels_from_input_set(input_set, sample_labels);
    
    int q = 0;
    
    for(unsigned int u = 0; u < _nb_stumps_per_label; u++) {
        if((_feature_sampling_period == 0 && u == 0) ||
                (_feature_sampling_period > 0  && u%_feature_sampling_period == 0)) {
            int f = 0;
            
            for(unsigned int h = 0; h < input_set->nbHeuristics(); h++) {
                for(unsigned int g = 0; g < _nb_features_sampled_per_heuristic; g++) {
                    heuristic_indexes[f] = h;
                    feature_indexes[f] = int(drand48() * input_set->nbFeatures(heuristic_indexes[f]));
                    f++;
                }
            }
            
        }
        
        for(int l = 0; l < _nb_labels; l++) {
            for(int n = 0; n < nb_samples; n++) {
                if(sample_labels[n] == l) {
                    edges[n] = individual_loss_derivative(  1.0, classifiers_responses[l][n]);
                } else {
                    edges[n] = individual_loss_derivative(- 1.0, classifiers_responses[l][n]);
                }
                my_sample_weights[n] = abs(edges[n]);
            }
            
            robust_sampling(nb_samples, my_sample_weights, _nb_samples_sampled_per_round, sample_indexes);
            
            compute_feature_responses(input_set, _nb_features_sampled_per_heuristic,
                    feature_indexes, feature_responses, sample_indexes, _nb_samples_sampled_per_round);
            
            unsigned int stump_feature = total_nb_features;
            
            pick_best_stump(_nb_samples_sampled_per_round,
                    total_nb_features,
                    classifiers_responses[l], edges,
                    feature_responses,
                    &stump_feature,
                    &_stump_thresholds[q],
                    sample_indexes);
            
            // Let's be a bit paranoid
            
            ASSERT(stump_feature < total_nb_features);
            ASSERT(q >= 0 && q < _nb_stumps_per_label * _nb_labels);
            
            _stump_labels[q] = l;
            _stump_heuristics[q] = heuristic_indexes[stump_feature];
            _stump_features[q] = feature_indexes[stump_feature];
            unsigned int best_feature[1];
            best_feature[0] = (unsigned int) feature_indexes[stump_feature];
            compute_best_feature_responses(input_set, best_feature_responses, best_feature,  (unsigned int) heuristic_indexes[stump_feature], nb_samples);
            
            compute_stump_weights(nb_samples, sample_labels, l,
                    stump_feature, _stump_thresholds[q],
                    classifiers_responses[l], best_feature_responses,
                    &_stump_weights0[q], &_stump_weights1[q]);
            
            
            
            for(int n = 0; n < nb_samples; n++) {
                if(best_feature_responses[n] >= _stump_thresholds[q]) {
                    classifiers_responses[l][n] += _stump_weights1[q];
                } else {
                    classifiers_responses[l][n] += _stump_weights0[q];
                }
            }
            
            outStream << "TRAINING_STUMP_CHOICE "
                    << q
                    << " label " << _stump_labels[q]
                    << " heuristic " << _stump_heuristics[q]
                    << " feature " << _stump_features[q]
                    << " threshold " << _stump_thresholds[q]
                    << " weight0 " << _stump_weights0[q]
                    << " weight1 " << _stump_weights1[q]
                    << endl;
            
            q++;
        }
        
        outStream << "TRAINING_LOSS "
                << u
                << " "
                << loss(nb_samples, _nb_labels, sample_labels, classifiers_responses)
                << endl;
        
        outStream << "TRAINING_NB_ERRORS "
                << u
                << " "
                << nb_errors(nb_samples, _nb_labels, sample_labels, classifiers_responses)
                << endl;
    }
    
    reorganize_stumps_by_blocks(input_set->nbHeuristics());
    
    delete[] edges;
    deallocate_array<scalar_t>(classifiers_responses);
    delete[] heuristic_indexes;
    delete[] feature_indexes;
    deallocate_array<scalar_t>(feature_responses);
    delete[] best_feature_responses;
    delete[] sample_labels;
    delete[] sample_indexes;
    
    return true;
}

bool SimpleAdaboost::classify(IClassifierInputSet* input_set,
        unsigned int image,
        const coordinates_t& position,
        tClassificationResults &results) {
    
    scalar_t *feature_responses = new scalar_t[_nb_stumps_per_label * _nb_labels];
    
    ASSERT(_nb_labels == input_set->nbLabels());
    ASSERT(_nb_features_from_heuristic);
    ASSERT(_stump_thresholds);
    ASSERT(_stump_weights0);
    ASSERT(_stump_weights1);
    ASSERT(_stump_heuristics);
    ASSERT(_stump_features);
    ASSERT(_stump_labels);
    
    int q = 0;
    for(unsigned int h = 0; h < input_set->nbHeuristics(); h++) {
        input_set->computeSomeFeatures(image,
                position,
                h,
                _nb_features_from_heuristic[h],
                _stump_features + q,
                feature_responses + q);
        q += _nb_features_from_heuristic[h];
    }
    
    for(unsigned int l = 0; l < _nb_labels; l++) {
        results[l] = 0;
    }
    
    for(unsigned int q = 0; q < _nb_stumps_per_label * _nb_labels; q++) {
        if(feature_responses[q] >= _stump_thresholds[q]) {
            results[_stump_labels[q]] += _stump_weights1[q];
        } else {
            results[_stump_labels[q]] += _stump_weights0[q];
        }
    }
    
    delete[] feature_responses;
    return true;
}

bool SimpleAdaboost::reportFeaturesUsed(tFeatureList &list) {
    for(unsigned int q = 0; q < _nb_labels * _nb_stumps_per_label; q++) {
        list.push_back(tFeature(_stump_heuristics[q],
                _stump_features[q]));
    }
    return true;
}

bool SimpleAdaboost::saveModel(PredictorModel &model) {
    // TODO: Implement it
    return true;
}


scalar_t SimpleAdaboost::robust_sampling(int nb, scalar_t *weights, int nb_to_sample, unsigned int *sampled) {
    ASSERT(nb > 0);
    if(nb == 1) {
        for(int k = 0; k < nb_to_sample; k++) sampled[k] = 0;
        ASSERT(!isnan(weights[0]));
        ASSERT(!isinf(weights[0]));
        return weights[0];
    } else {
        scalar_t *pair_weights = new scalar_t[(nb+1)/2];
        for(int k = 0; k < nb/2; k++) {
            pair_weights[k] = weights[2 * k] + weights[2 * k + 1];
            ASSERT(!isnan(pair_weights[k]) && !isinf(pair_weights[k]));
        }
        if(nb%2) {
            pair_weights[(nb+1)/2 - 1] = weights[nb-1];
            ASSERT(!isnan(pair_weights[(nb+1)/2 - 1]) && !isinf(pair_weights[(nb+1)/2 - 1]));
        }
        scalar_t result = robust_sampling((nb+1)/2, pair_weights, nb_to_sample, sampled);
        for(int k = 0; k < nb_to_sample; k++) {
            unsigned int s = sampled[k];
            // There is a bit of a trick for the isolated sample in the odd
            // case. Since the corresponding pair weight is the same as the
            // one sample alone, the test is always true and the isolated
            // sample will be taken for sure.
            if(drand48() * pair_weights[s] <= weights[2 * s])
                sampled[k] = 2 * s;
            else
                sampled[k] = 2 * s + 1;
        }
        delete[] pair_weights;
        return result;
    }
}

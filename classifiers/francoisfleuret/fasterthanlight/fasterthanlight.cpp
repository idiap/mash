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

    This is a version of the 'Tasting' procedure. We have a paper
    under blind review at CVPR 2011, please do *NOT* make this source
    visible by the public before the method is published somewhere.

    This predictor is similar to SimpleAdaboost. However, it uses a
    Tasting procedure to improve the speed.

    More precisely:

    (1) When the learning starts, it samples NB_FEATURES_FOR_TASTING
    feature from every heuristic, and stores their values on every
    sample

    (2) It then loops NB_STUMP_BLOCKS, and at the beginning of each
    block, using the stored responses, it computes what is the
    heuristic that would reduce the loss the most overall. It samples,
    NB_FEATURES_FOR_OPTIMIZING feature from that heuristic.

    (3) It then loops NB_STUMPS_PER_BLOCKS, and in each loop it picks
    the best feature by looking at the NB_FEATURES_FOR_OPTIMIZING that
    has been stored for that block.


*/

#include <MLInputSet/MashInputSet.h>
#include <mash-classification/classifier.h>

#include <cmath>

using namespace Mash;
using namespace std;

#include "misc.h"
#include "fusion_sort.h"

//////////////////////////////////////////////////////////////////////

void compute_feature_responses(IClassifierInputSet *input_set,
                               unsigned int nb_features,
                               unsigned int *heuristic_indexes,
                               unsigned int *feature_indexes,
                               scalar_t **result_feature_responses) {
  const unsigned int nb_heuristics = input_set->nbHeuristics();
  unsigned int *features_counters_per_heuristic;
  unsigned int **feature_indexes_per_heuristics;

  features_counters_per_heuristic = new unsigned int[nb_heuristics];
  feature_indexes_per_heuristics = new unsigned int *[nb_heuristics];

  for(int h = 0; h < nb_heuristics; h++) {
    features_counters_per_heuristic[h] = 0;
  }
  for(int f = 0; f < nb_features; f++) {
    features_counters_per_heuristic[heuristic_indexes[f]]++;
  }
  for(int h = 0; h < nb_heuristics; h++) {
    feature_indexes_per_heuristics[h] =
      new unsigned int[features_counters_per_heuristic[h]];
    features_counters_per_heuristic[h] = 0;
  }
  for(int f = 0; f < nb_features; f++) {
    unsigned int h = heuristic_indexes[f];
    feature_indexes_per_heuristics[h][features_counters_per_heuristic[h]++] =
      feature_indexes[f];
  }

  int s = 0, t;
  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    for(unsigned int h = 0; h < nb_heuristics; h++) {
      t = s;
      int k = 0;
      for(unsigned int o = 0; o < objects.size(); o++) {
        input_set->computeSomeFeatures(i,
                                       objects[o].roi_position,
                                       h,
                                       features_counters_per_heuristic[h],
                                       feature_indexes_per_heuristics[h],
                                       result_feature_responses[t] + k);
        t++;
        k += features_counters_per_heuristic[h];
      }
    }
    s = t;
  }

#warning the results are still "by hblocks", hence not ordered properly

  for(int h = 0; h < nb_heuristics; h++) {
    delete[] feature_indexes_per_heuristics[h];
  }

  delete[] features_counters_per_heuristic;
  delete[] feature_indexes_per_heuristics;
}


inline scalar_t individual_loss(scalar_t label, scalar_t response) {
  return exp( - label * response);
}

inline scalar_t individual_loss_derivative(scalar_t label, scalar_t response) {
  return - label * exp( - label * response);
}

class FasterThanLight: public Classifier
{
public:
  FasterThanLight();
  virtual ~FasterThanLight();

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
  unsigned int _nb_labels, _nb_heuristics;

  unsigned int _nb_stump_blocks;
  unsigned int _nb_features_for_tasting;
  unsigned int _nb_features_for_optimizing;
  unsigned int _nb_stumps_per_block;

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

  void compute_heuristic_feature_response_blocks(IClassifierInputSet *input_set,
                                 unsigned int nb_features_sampled_per_heuristic,
                                 unsigned int *feature_index_blocks,
                                 scalar_t **result_feature_responses);

  void compute_one_feature_response_block(IClassifierInputSet *input_set,
                                          unsigned int heuristic,
                                          unsigned int nb_features,
                                          unsigned int *feature_indexes,
                                          scalar_t **result_feature_responses);

  scalar_t loss(unsigned int nb_samples, unsigned int nb_labels,
                unsigned int *sample_labels, scalar_t **responses);

  unsigned int nb_errors(unsigned int nb_samples, unsigned int nb_labels,
                         unsigned int *sample_labels, scalar_t **responses);

  scalar_t compute_bounded_weight(scalar_t num, scalar_t den);

  void compute_stump_weights(unsigned int nb_samples, unsigned int *sample_labels,
                             unsigned int positive_label,
                             unsigned int stump_feature, scalar_t stump_threshold,
                             scalar_t *classifier_responses, scalar_t **feature_responses,
                             scalar_t *result_weight0, scalar_t *result_weight1);

  scalar_t pick_best_stump(unsigned int nb_samples,
                           unsigned int nb_features,
                           scalar_t *responses, scalar_t *edges,
                           scalar_t **feature_responses,
                           unsigned int *result_feature, scalar_t *result_threshold);

  void reorganize_stumps_by_blocks(int nb_heuristics);
};

extern "C" Classifier* new_classifier() {
  return new FasterThanLight();
}

/************************* CONSTRUCTION / DESTRUCTION *************************/

FasterThanLight::FasterThanLight() {
  _nb_stump_blocks = 10;
  _nb_features_for_tasting = 100;
  _nb_features_for_optimizing = 100;
  _nb_stumps_per_block = 10;

  _stump_thresholds = 0;
  _stump_weights0 = 0;
  _stump_weights1 = 0;

  _stump_heuristics = 0;
  _stump_features = 0;
  _stump_labels = 0;
}

FasterThanLight::~FasterThanLight() {
  free();
}

void FasterThanLight::free() {
  delete[] _nb_features_from_heuristic;

  delete[] _stump_thresholds;
  delete[] _stump_weights0;
  delete[] _stump_weights1;

  delete[] _stump_heuristics;
  delete[] _stump_features;
  delete[] _stump_labels;
}

void FasterThanLight::allocate(int nb_heuristics) {
  _nb_features_from_heuristic = new unsigned int[nb_heuristics];

  _stump_thresholds = new scalar_t[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];
  _stump_weights0 = new scalar_t[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];
  _stump_weights1 = new scalar_t[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];

  _stump_heuristics = new unsigned int[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];
  _stump_features =  new unsigned int[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];
  _stump_labels =  new unsigned int[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];
}

/************************* IMPLEMENTATION OF Classifier ***********************/

bool FasterThanLight::setup(const tExperimentParametersList& parameters) {
  _nb_stump_blocks = 10;
  _nb_features_for_tasting = 100;
  _nb_features_for_optimizing = 100;
  _nb_stumps_per_block = 10;


  for(tExperimentParametersIterator iter = parameters.begin();
      iter != parameters.end(); ++iter) {

    if(iter->first == "NB_STUMP_BLOCKS") {
      _nb_stump_blocks = iter->second.getInt(0);
      outStream << "SETUP NB_STUMP_BLOCKS "
                << _nb_stump_blocks
                << endl;
    }

    else if(iter->first == "NB_FEATURES_FOR_TASTING") {
      _nb_features_for_tasting = iter->second.getInt(0);
      outStream << "SETUP NB_FEATURES_FOR_TASTING "
                << _nb_features_for_tasting
                << endl;
    }

    else if(iter->first == "NB_FEATURES_FOR_OPTIMIZING") {
      _nb_features_for_optimizing = iter->second.getInt(0);
      outStream << "SETUP NB_FEATURES_FOR_OPTIMIZING "
                << _nb_features_for_optimizing
                << endl;
    }

    else if(iter->first == "NB_STUMPS_PER_BLOCKS") {
      _nb_stumps_per_block = iter->second.getInt(0);
      outStream << "SETUP NB_STUMPS_PER_BLOCKS "
                << _nb_stumps_per_block
                << endl;
    }

    else {
      outStream << "Warning: unknown parameter '" << iter->first << "'."
                << std::endl;
    }
  }

  return true;
}

bool FasterThanLight::loadModel(PredictorModel &model, DataReader &internal_data)
{
    // TODO: Implement it
    return true;
}

scalar_t FasterThanLight::compute_bounded_weight(scalar_t num, scalar_t den) {
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

scalar_t FasterThanLight::pick_best_stump(unsigned int nb_samples,
                                          unsigned int nb_features,
                                          scalar_t *responses, scalar_t *edges,
                                          scalar_t **feature_responses,
                                          unsigned int *result_feature,
                                          scalar_t *result_threshold) {

  int *indexes = new int[nb_samples];
  int *sorted_indexes = new int[nb_samples];
  scalar_t *one_feature_responses = new scalar_t[nb_samples];

  scalar_t max_loss_derivative = 0;
  *result_feature = -1;

  for(int f = 0; f < nb_features; f++) {
    scalar_t s = 0;

    for(int n = 0; n < nb_samples; n++) {
      one_feature_responses[n] = feature_responses[n][f];
      indexes[n] = n;
      s += edges[n];
    }

    indexed_fusion_sort(nb_samples, indexes, sorted_indexes, one_feature_responses);

    for(int n = 0; n < nb_samples - 1; n++) {
      int i = sorted_indexes[n];
      int j = sorted_indexes[n + 1];
      s -= 2 * edges[i];
      if(one_feature_responses[j] > one_feature_responses[i]) {
        if(abs(s) > abs(max_loss_derivative)) {
          max_loss_derivative = s;
          *result_feature = f;
          *result_threshold = (one_feature_responses[i] +
                               one_feature_responses[j]) / 2;
        }
      }
    }
  }

  ASSERT(*result_threshold >= 0);

  delete[] one_feature_responses;
  delete[] indexes;
  delete[] sorted_indexes;

  return max_loss_derivative;
}

void FasterThanLight::reorganize_stumps_by_blocks(int nb_heuristics) {
  int n = 0, pred_n = 0;

  for(unsigned int h = 0; h < nb_heuristics; h++) {
    for(unsigned int q = 0; q < _nb_labels * _nb_stump_blocks * _nb_stumps_per_block; q++) {
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

void FasterThanLight::compute_heuristic_feature_response_blocks(IClassifierInputSet *input_set,
                                                unsigned int nb_features_per_heuristic,
                                                unsigned int *feature_index_blocks,
                                                scalar_t **result_feature_responses) {

  int s = 0, t;
  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    for(unsigned int h = 0; h < input_set->nbHeuristics(); h++) {
      t = s;
      for(unsigned int o = 0; o < objects.size(); o++) {
        input_set->computeSomeFeatures(i,
                                       objects[o].roi_position,
                                       h,
                                       nb_features_per_heuristic,
                                       feature_index_blocks + h * nb_features_per_heuristic,
                                       result_feature_responses[t] + h * nb_features_per_heuristic);
        t++;
      }
    }
    s = t;
  }
}

void FasterThanLight::compute_one_feature_response_block(IClassifierInputSet *input_set,
                                                         unsigned int heuristic,
                                                         unsigned int nb_features,
                                                         unsigned int *feature_indexes,
                                                         scalar_t **result_feature_responses) {

  int s = 0;
  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    for(unsigned int o = 0; o < objects.size(); o++) {
      input_set->computeSomeFeatures(i,
                                     objects[o].roi_position,
                                     heuristic,
                                     nb_features,
                                     feature_indexes,
                                     result_feature_responses[s]);
      s++;
    }
  }
}

scalar_t FasterThanLight::loss(unsigned int nb_samples, unsigned int nb_labels,
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

unsigned int FasterThanLight::nb_errors(unsigned int nb_samples, unsigned int nb_labels,
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

void FasterThanLight::compute_stump_weights(unsigned int nb_samples, unsigned int *sample_labels,
                                            unsigned int positive_label,
                                            unsigned int stump_feature, scalar_t stump_threshold,
                                            scalar_t *classifier_responses, scalar_t **feature_responses,
                                            scalar_t *result_weight0, scalar_t *result_weight1) {

  scalar_t num0 = 0, den0 = 0, num1 = 0, den1 = 0;

  for(int n = 0; n < nb_samples; n++) {
    if(feature_responses[n][stump_feature] >= stump_threshold) {
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

unsigned int FasterThanLight::nb_samples_in_input_set(IClassifierInputSet *input_set) {
  unsigned int nb_samples = 0;
  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    nb_samples += objects.size();
  }
  return nb_samples;
}

void FasterThanLight::collect_labels_from_input_set(IClassifierInputSet *input_set,
                                                    unsigned int *labels) {

  int s = 0;

  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    for(unsigned int o = 0; o < objects.size(); o++) {
      labels[s++] = objects[o].label;
    }
  }
}

bool FasterThanLight::train(IClassifierInputSet* input_set, scalar_t &train_error) {
  int nb_samples;

  scalar_t **tasting_feature_responses;
  unsigned int *tasting_heuristic_indexes;
  unsigned int *tasting_feature_indexes;
  scalar_t **feature_responses;
  unsigned int *heuristic_indexes;
  unsigned int *feature_indexes;
  scalar_t **classifiers_responses;
  scalar_t *edges;
  unsigned int *sample_labels;

  free();

  _nb_labels = input_set->nbLabels();
  _nb_heuristics = input_set->nbHeuristics();

  allocate(input_set->nbHeuristics());

  // Compute the total number of samples

  nb_samples = nb_samples_in_input_set(input_set);

  outStream << "TRAINING nb_samples "
            << nb_samples
            << " _nb_labels "
            << _nb_labels
            << endl;

  tasting_feature_responses = allocate_array<scalar_t>(nb_samples * _nb_heuristics, _nb_features_for_tasting);
  tasting_heuristic_indexes = new unsigned int[_nb_features_for_tasting * _nb_heuristics];
  tasting_feature_indexes = new unsigned int[_nb_features_for_tasting * _nb_heuristics];

  feature_responses = allocate_array<scalar_t>(nb_samples, _nb_features_for_optimizing);
  heuristic_indexes = new unsigned int[_nb_features_for_optimizing];
  feature_indexes = new unsigned int[_nb_features_for_optimizing];
  classifiers_responses = allocate_clear_array<scalar_t>(_nb_labels, nb_samples);
  edges = new scalar_t[nb_samples];
  sample_labels = new unsigned int[nb_samples];

  collect_labels_from_input_set(input_set, sample_labels);

  int q = 0;

  int f = 0;
  for(unsigned int h = 0; h < _nb_heuristics; h++) {
    for(unsigned int g = 0; g < _nb_features_for_tasting; g++) {
      tasting_heuristic_indexes[f] = h;
      tasting_feature_indexes[f] = int(drand48() * input_set->nbFeatures(heuristic_indexes[f]));
      f++;
    }
  }

  outStream << "COMPUTING_TASTING_FEATURE_RESPONSES" << endl;

  compute_heuristic_feature_response_blocks(input_set, _nb_features_for_tasting,
                                            tasting_feature_indexes, tasting_feature_responses);

  for(unsigned int u = 0; u < _nb_stump_blocks * _nb_stumps_per_block; u++) {
    if(u % _nb_stumps_per_block == 0) {
      unsigned int tasting_feature, tasting_heuristic;
      scalar_t tasting_threshold;

      tasting_heuristic = 0;

      scalar_t best_sum_loss_derivative = 0, sum_loss_derivative;

      for(int h = 0; h < _nb_heuristics; h++) {
        sum_loss_derivative = 0;
        for(int l = 0; l < _nb_labels; l++) {
          for(int n = 0; n < nb_samples; n++) {
            if(sample_labels[n] == l) {
              edges[n] = individual_loss_derivative(  1.0, classifiers_responses[l][n]);
            } else {
              edges[n] = individual_loss_derivative(- 1.0, classifiers_responses[l][n]);
            }
          }

          sum_loss_derivative += abs(pick_best_stump(nb_samples,
                                                     _nb_heuristics * _nb_features_for_tasting,
                                                     classifiers_responses[l], edges,
                                                     tasting_feature_responses + nb_samples * h,
                                                     &tasting_feature,
                                                     &tasting_threshold));
        }

        outStream << "TASTING " << h << " " << sum_loss_derivative << endl;

        if(sum_loss_derivative > best_sum_loss_derivative) {
          tasting_heuristic = h;
          best_sum_loss_derivative = sum_loss_derivative;
        }
      }

      for(unsigned int f = 0; f < _nb_features_for_optimizing; f++) {
        heuristic_indexes[f] = tasting_heuristic;
        feature_indexes[f] = int(drand48() * input_set->nbFeatures(tasting_heuristic));
      }

      outStream << "COMPUTING_OPTIMIZING_FEATURE_RESPONSES " << tasting_heuristic << endl;

      compute_one_feature_response_block(input_set,
                                         tasting_heuristic,
                                         _nb_features_for_optimizing,
                                         feature_indexes, feature_responses);
    }

    for(int l = 0; l < _nb_labels; l++) {
      unsigned int stump_feature;

      for(int n = 0; n < nb_samples; n++) {
        if(sample_labels[n] == l) {
          edges[n] = individual_loss_derivative(  1.0, classifiers_responses[l][n]);
        } else {
          edges[n] = individual_loss_derivative(- 1.0, classifiers_responses[l][n]);
        }
      }

      pick_best_stump(nb_samples,
                      _nb_features_for_optimizing,
                      classifiers_responses[l], edges,
                      feature_responses,
                      &stump_feature,
                      &_stump_thresholds[q]);

      _stump_labels[q] = l;
      _stump_heuristics[q] = heuristic_indexes[stump_feature];
      _stump_features[q] = feature_indexes[stump_feature];

      compute_stump_weights(nb_samples, sample_labels, l,
                            stump_feature, _stump_thresholds[q],
                            classifiers_responses[l], feature_responses,
                            &_stump_weights0[q], &_stump_weights1[q]);

      for(int n = 0; n < nb_samples; n++) {
        if(feature_responses[n][stump_feature] >= _stump_thresholds[q]) {
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
  delete[] sample_labels;
  deallocate_array<scalar_t>(tasting_feature_responses);
  delete[] tasting_heuristic_indexes;
  delete[] tasting_feature_indexes;


  return true;
}

bool FasterThanLight::classify(IClassifierInputSet* input_set,
                               unsigned int image,
                               const coordinates_t& position,
                               tClassificationResults &results) {
  scalar_t *feature_responses = new scalar_t[_nb_stump_blocks * _nb_stumps_per_block * _nb_labels];

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

  for(unsigned int q = 0; q < _nb_stump_blocks * _nb_stumps_per_block * _nb_labels; q++) {
    if(feature_responses[q] >= _stump_thresholds[q]) {
      results[_stump_labels[q]] += _stump_weights1[q];
    } else {
      results[_stump_labels[q]] += _stump_weights0[q];
    }
  }

  delete[] feature_responses;
  return true;
}

bool FasterThanLight::reportFeaturesUsed(tFeatureList &list) {
  for(unsigned int q = 0; q <  _nb_stump_blocks * _nb_stumps_per_block * _nb_labels; q++) {
    list.push_back(tFeature(_stump_heuristics[q],
                            _stump_features[q]));
  }
  return true;
}

bool FasterThanLight::saveModel(PredictorModel &model)
{
    // TODO: Implement it
    return true;
}

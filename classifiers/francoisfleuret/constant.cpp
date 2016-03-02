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

    This classifier does not use the features at all, and just returns
    the prior distribution as a prediction.

*/

#include <MLInputSet/MashInputSet.h>
#include <mash-classification/classifier.h>

#include <cmath>

using namespace Mash;
using namespace std;

//////////////////////////////////////////////////////////////////////

class Constant: public Classifier
{
public:
  Constant();
  virtual ~Constant();

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
  scalar_t *_prior;
};

extern "C" Classifier* new_classifier() {
  return new Constant();
}

/************************* CONSTRUCTION / DESTRUCTION *************************/

Constant::Constant() {
  _prior = 0;
}

Constant::~Constant() {
  delete[] _prior;
}

/************************* IMPLEMENTATION OF Classifier ***********************/

bool Constant::setup(const tExperimentParametersList& parameters) {
  for(tExperimentParametersIterator iter = parameters.begin();
      iter != parameters.end(); ++iter) {
    if(iter->first == "MEMORY") {
      size_t nb_megs_to_allocate = iter->second.getInt(0);
      char *blah;

      outStream << "ALLOCATING " << nb_megs_to_allocate << " MEGAS" << endl;
      blah = new char[nb_megs_to_allocate * 1024 * 1024];

      for(int i = 0; i < nb_megs_to_allocate * 1024 * 1024; i++) {
        blah[i] = i%10;
      }

      outStream << "DEALLOCATING" << endl;
      delete[] blah;

      outStream << "FINE" << endl;
    }
    else {
      outStream << "Warning: unknown parameter '" << iter->first << "'."
                << std::endl;
    }
  }

  return true;
}

bool Constant::loadModel(PredictorModel &model, DataReader &internal_data)
{
    // TODO: Implement it
    return true;
}

bool Constant::train(IClassifierInputSet* input_set, scalar_t &train_error) {
  delete[] _prior;
  _nb_labels = input_set->nbLabels();
  _prior = new scalar_t[_nb_labels];

  for(int l = 0; l < _nb_labels; l++) {
    _prior[l] = 0;
  }

  scalar_t total = 0.0;
  for(unsigned int i = 0; i < input_set->nbImages(); i++) {
    tObjectsList objects;
    input_set->objectsInImage(i, &objects);
    for(unsigned int o = 0; o < objects.size(); o++) {
      _prior[objects[o].label] += 1.0;
      total += 1.0;
    }
  }

  if(total > 0) {
    for(int l = 0; l < _nb_labels; l++) {
      _prior[l] /= total;
      outStream << "TRAINING_PROBA " << l << " " << _prior[l] << endl;
    }
    return true;
  } else {
    return false;
  }
}

bool Constant::classify(IClassifierInputSet* input_set,
                        unsigned int image,
                        const coordinates_t& position,
                        tClassificationResults &results) {
  for(unsigned int l = 0; l < _nb_labels; l++) {
    results[l] = _prior[l];
  }
  return true;
}

bool Constant::reportFeaturesUsed(tFeatureList &list) {
  return true;
}

bool Constant::saveModel(PredictorModel &model)
{
    // TODO: Implement it
    return true;
}

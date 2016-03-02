/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Charles Dubout (charles.dubout@idiap.ch)
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


//------------------------------------------------------------------------------
/// @file MLClassifiers/Perceptron.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.05
/// @version 1.5
//------------------------------------------------------------------------------

#include "Perceptron.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>

using namespace ML;

Perceptron::Perceptron(bool stochastic,
					   scalar_t decay,
					   unsigned int nbSteps)
: stochastic_(stochastic), decay_(decay), nbSteps_(nbSteps) {
	// The learning rate must be in the range (0, 1]
	assert(decay > 0 && decay < 1);

	// The number of steps must be strictly positive
	assert(nbSteps);
}

Classifier* Perceptron::clone() const {
	return new Perceptron(*this);
}

void Perceptron::train(InputSet& inputSet) {
	// Get the number of samples, features and labels
	// Add one dummy feature always equal to 1
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = 1 + inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Delete the previous problem
	w_.clear();

	// Get the labels of the samples
	const unsigned int* labels = inputSet.labels();

	// Get the weights of the samples
	const scalar_t* weights = inputSet.weights();

	// Initialise the normal vectors to random values between 0 and 1 (as
	// required by the Pocket algorithm)
	std::vector<scalar_t> w(nbLabels * nbFeatures);

	for(unsigned int i = 0; i < w.size(); ++i) {
		w[i] = std::rand() / scalar_t(RAND_MAX);
	}

	// Assume at the beginning that every sample is completely missclassified
	unsigned int nbErrors = nbSamples * nbLabels;

	// Begin with a rho value of 1
	scalar_t rho = 1;

	// The current iteration
	unsigned int t = 1;

	// The number of iterations without update of the pocket
	unsigned int i = 0;

	// Gradients vector (combined update of the normal vectors)
	std::vector<scalar_t> gradients_;

	// The stochastic variant doesn't need gradients
	if(!stochastic_) {
		gradients_.resize(nbLabels * nbFeatures);
	}

	std::vector<unsigned int> indices(nbSamples);

	for(unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;
	}

	for(;;) {
		// Gradients vector (combined update of the normal vectors)
		std::vector<scalar_t>& gradients = stochastic_ ? w : gradients_;

		// Number of missclassifications
		unsigned int nbMissclassifications = 0;

		// For each sample
		for(unsigned int s = 0; s < nbSamples; ++s) {
			// Get the features associated with the sample
			const scalar_t* features = inputSet.features(indices[s]);
			--features;

			// Compute the response of the real label
			const unsigned int label = labels[indices[s]];
			scalar_t response = w[label * nbFeatures];

			for(unsigned int f = 1; f < nbFeatures; ++f) {
				response += features[f] * w[label * nbFeatures + f];
			}

			// Compute the response of the other labels
			scalar_t weight = rho * weights[indices[s]];

			for(unsigned int l = 0; l < nbLabels; ++l) {
				if(l != label) {
					// Computes the dot product of the two vectors
					scalar_t dot = w[l * nbFeatures];

					for(unsigned int f = 1; f < nbFeatures; ++f) {
						dot += features[f] * w[l * nbFeatures + f];
					}

					// Update the gradients in case of error (dot >= response)
					if(dot >= response) {
						// Shift the normal vector associated with the real
						// label toward the sample
						gradients[label * nbFeatures] += weight;

						for(unsigned int f = 1; f < nbFeatures; ++f) {
							gradients[label * nbFeatures + f] += weight *
																 features[f];
						}

						// Shift the normal vector associated with the predicted
						// label outward the sample
						gradients[l * nbFeatures] -= weight;

						for(unsigned int f = 1; f < nbFeatures; ++f) {
							gradients[l * nbFeatures + f] -= weight *
															 features[f];
						}

						// Increment the number of missclassifications
						++nbMissclassifications;
					}
				}
			}
		}

		// If the number of errors is lower or equal than previously, update the
		// pocket
		if(t > nbSteps_) {
			if(nbMissclassifications < nbErrors) {
				w_ = w;
				nbErrors = nbMissclassifications;
				i = 0;
			}
			else {
				if(nbMissclassifications == nbErrors) {
					w_ = w;
				}

				++i;
			}
		}

		std::cout << "t = " << t << ", convergence = "
				  << (i * 100.0f / nbSteps_) << "%, #missclassifications =  "
				  << nbMissclassifications << std::endl;

		// Break if there was nbSteps_ iterations without an update of the
		// pocket
		if(i == nbSteps_) {
			break;
		}

		// Update the normal vectors if needed
		if(!stochastic_) {
			std::transform(w.begin(), w.end(), gradients_.begin(), w.begin(),
						   std::plus<scalar_t>());
		}

		// Increment the iteration counter
		++t;

		// Decrease rho
		rho *= decay_;

		// Shuffle the sample indices
		std::random_shuffle(indices.begin(), indices.end());
	}
}

void Perceptron::distribution(InputSet& inputSet,
							  unsigned int sample,
							  scalar_t* distr) const {
	// Get the number of features and labels
	const unsigned int nbFeatures = 1 + inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Get the features of the sample to classifiy
	const scalar_t* features = inputSet.features(sample);

	// Map them to the same ranges as the training samples
	for(unsigned int l = 0; l < nbLabels; ++l) {
		// Computes the dot product of the two vectors
		distr[l] = std::inner_product(features, features + nbFeatures - 1,
									  &w_[l * nbFeatures + 1],
									  w_[l * nbFeatures]);
	}
}

void Perceptron::print(std::ostream& out) const {
	out << "Perceptron classifier ("
		<< (stochastic_ ? "stochastic" : "standard") << ", decay: " << decay_
		<< ", steps: " << nbSteps_ << ")."<< std::endl;
}

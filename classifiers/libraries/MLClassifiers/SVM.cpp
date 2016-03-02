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
/// @file MLClassifiers/SVM.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.29
/// @version 2.1
//------------------------------------------------------------------------------

#include "SVM.h"

#include <Eigen/Core>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>

using namespace ML;

inline scalar_t nrm2(int n, const scalar_t* x, int incX) {
	// Warning! Assume incX == 1!
	Eigen::Map<Eigen::Matrix<scalar_t, Eigen::Dynamic, 1> > mx(x, n);
	return mx.norm();
}

//void svm_print_null(const char*) {
	// Do nothing
//}

SVM::SVM(bool nusvm,
		 Kernel kernel,
		 unsigned int degree,
		 scalar_t gamma,
		 scalar_t coef0,
		 scalar_t cnu)
: model_(0) {
	// Make sure that cnu is below one if the SVM is a nu-SVM
	assert(!nusvm || cnu <= 1);

	// Set parameters
	parameters_.svm_type = nusvm ? NU_SVC : C_SVC;
	parameters_.kernel_type = static_cast<int>(kernel);
	parameters_.degree = degree;
	parameters_.gamma = (parameters_.kernel_type == LINEAR) ? 1 : gamma;
	parameters_.coef0 = coef0;

	// Uses libsvm default values
	parameters_.cache_size = 100;
	parameters_.eps = 1e-3;
	parameters_.C = cnu;
	parameters_.nr_weight = 0;
	parameters_.weight_label = 0;
	parameters_.weight = 0;
	parameters_.nu = cnu;
	parameters_.p = 0;
	parameters_.shrinking = 1;
	parameters_.probability = 0;

//	svm_set_print_string_function(&svm_print_null);
}

SVM::~SVM() {
	// Delete the model
	if(model_) {
		svm_free_and_destroy_model(&model_);
	}
}

SVM::SVM(const SVM& svm) : parameters_(svm.parameters_), model_(0) {
	// Nothing else to copy
}

SVM& SVM::operator=(const SVM& svm) {
	// Delete the model
	if(model_) {
		svm_free_and_destroy_model(&model_);
	}

	// Delete the problem
	matrix_.clear();
	samples_.clear();
	labels_.clear();

	// Delete the labels mapping
	map_.clear();

	// Recopy only the parameters
	parameters_ = svm.parameters_;

	return *this;
}

Classifier* SVM::clone() const {
	return new SVM(*this);
}

void SVM::train(InputSet& inputSet) {
	// Get the number of features, samples and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Delete the previous model
	if(model_) {
		svm_free_and_destroy_model(&model_);
	}

	// Delete the previous problem
	matrix_.clear();
	samples_.clear();
	labels_.clear();

	// Recopy the number of samples
	problem_.l = nbSamples;

	// Recopy the labels
	const unsigned int* labels = inputSet.labels();
	labels_.assign(labels, labels + nbSamples);
	problem_.y = &labels_[0];

	// Recopy the features
	inputSet.swapFeatures(matrix_);
	samples_.resize(nbSamples);
	problem_.x = &samples_[0];

	// Compute the mean norm
	scalar_t meanNorm = 0;

	for(unsigned int s = 0; s < nbSamples; ++s) {
		samples_[s].dim = nbFeatures;
		samples_[s].values = &matrix_[s * nbFeatures];

		// Add the mean norm of that sample
		meanNorm += nrm2(samples_[s].dim, samples_[s].values, 1);
	}

	// Divide the sum of the norms by the number of samples
	meanNorm /= nbSamples;

	std::cout << "[SVM::train] mean(norm): " << meanNorm << '.' << std::endl;

	// Rescale the features so that their mean norm is 1 in case of linear
	// kernel
	if(parameters_.kernel_type == LINEAR) {
		std::transform(matrix_.begin(), matrix_.end(), matrix_.begin(),
					   std::bind2nd(std::divides<scalar_t>(), meanNorm));
	}

	// Make sure that the parameters are correct
	bool crossValidateGamma = parameters_.gamma < 0 &&
							  parameters_.kernel_type != LINEAR;
	bool crossValidateCnu = parameters_.C < 0;

	// Sets gamma and C/nu to default values in order to pass the parameter
	// check
	if(crossValidateGamma) {
		parameters_.gamma = 1.0 / (meanNorm * meanNorm);
	}

	if(crossValidateCnu) {
		parameters_.C = 1000;
		parameters_.nu = 1.0 / nbSamples;
	}

	// There is a problem with the parameters
	assert(!svm_check_parameter(&problem_, &parameters_));

	// If gamma is below zero, use 5-folds cross-validation to determine it
	// (uses C = 1000 or nu = 1 / nbSamples in case they need to be
	// cross-validated also)
	unsigned int nbErrorsMin = nbSamples + 1; // Initialize past the maximum

	if(crossValidateGamma) {
		std::vector<double> target(nbSamples); // The predicted labels

		/// Make the assumption that the optimal gamma is in this range
		const scalar_t min = parameters_.gamma / 32;
		const scalar_t max = parameters_.gamma * 32;

		for(parameters_.gamma = max; parameters_.gamma > min;
			parameters_.gamma *= 0.5) {
			svm_cross_validation(&problem_, &parameters_, 5, &target[0]);

			// Count the number of errors
			unsigned int nbErrors = 0;

			for(unsigned int s = 0; s < nbSamples; ++s) {
				if(target[s] != labels[s]) {
					++nbErrors;
				}
			}

			std::cout << "[SVM::train] 5 folds cross-validation error for "
						 "gamma = " << parameters_.gamma << ": "
					  << nbErrors * 100.0f / nbSamples << "%." << std::endl;

			// The new gamma is better than the previous one
			if(nbErrors < nbErrorsMin) {
				nbErrorsMin = nbErrors;
			}
			// The optimal gamma was found
			else {
				break;
			}
		}

		// gamma got multiplied one time too much
		parameters_.gamma /= 0.5;

		// Print gamma to the log
		std::cout << "[SVM::train] optimal gamma as determined by 5 folds "
					 "cross-validation: " << parameters_.gamma << '.'
				  << std::endl;
	}

	// If C/nu is below zero, use 5-folds cross-validation to determine it
	if(crossValidateCnu && parameters_.svm_type == C_SVC) {
		std::vector<double> target(nbSamples); // The predicted labels

		for(parameters_.C = crossValidateGamma ? 100 : 1000;
			parameters_.C >= 0.01; parameters_.C /= 10) {
			svm_cross_validation(&problem_, &parameters_, 5, &target[0]);

			// Count the number of errors
			unsigned int nbErrors = 0;

			for(unsigned int s = 0; s < nbSamples; ++s) {
				if(target[s] != labels[s]) {
					++nbErrors;
				}
			}

			std::cout << "[SVM::train] 5 folds cross-validation error for C = "
					  << parameters_.C << ": " << nbErrors * 100.0f / nbSamples
					  << "%." << std::endl;

			// The new C is better than the previous one
			if(nbErrors <= nbErrorsMin) {
				nbErrorsMin = nbErrors;
			}
			// The optimal C was found
			else {
				break;
			}
			
		}

		// C got divided one time too much
		parameters_.C *= 10;

		// Print C to the log
		std::cout << "[SVM::train] optimal C as determined by 5 folds cross-"
					 "validation: " << parameters_.C << '.' << std::endl;
	}
	else if(crossValidateCnu && parameters_.svm_type == NU_SVC) {
		std::vector<double> target(nbSamples); // The predicted labels

		for(parameters_.nu = (crossValidateGamma ? 2.0 : 1.0) / nbSamples;
			parameters_.nu <= 0.5; parameters_.nu *= 2) {
			svm_cross_validation(&problem_, &parameters_, 5, &target[0]);

			// Count the number of errors
			unsigned int nbErrors = 0;

			for(unsigned int s = 0; s < nbSamples; ++s) {
				if(target[s] != labels[s]) {
					++nbErrors;
				}
			}

			std::cout << "[SVM::train] 5 folds cross-validation error for nu = "
					  << parameters_.nu << ": " << nbErrors * 100.0f / nbSamples
					  << "%." << std::endl;

			// The new nu is better than the previous one
			if(nbErrors < nbErrorsMin) {
				nbErrorsMin = nbErrors;
			}
			// The optimal nu was found
			else {
				break;
			}
		}

		// nu got multiplied one time too much
		parameters_.nu /= 2;

		// Print nu to the log
		std::cout << "[SVM::train] optimal nu as determined by 5 folds cross-"
					 "validation: " << parameters_.nu << '.' << std::endl;
	}

	// Train the svm
	model_ = svm_train(&problem_, &parameters_);
	assert(model_);

	// Reset gamma and C/nu so that they will be cross-validated again
	if(crossValidateGamma) {
		parameters_.gamma = -1;
	}

	if(crossValidateCnu) {
		parameters_.C = parameters_.nu = -1;
	}

	// Save libsvm labels
	map_.clear();
	map_.resize(nbLabels);
	svm_get_labels(model_, &map_[0]);
}

void SVM::distribution(InputSet& inputSet,
					   unsigned int sample,
					   scalar_t* distr) const {
	// Get the number of features and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Make sure that we have a model
	assert(model_);

	// Make sure that there is the same number of labels
	assert(static_cast<unsigned int>(svm_get_nr_class(model_)) == nbLabels);

	// Zero-out the distribution
	std::fill(distr, distr + nbLabels, 0);

	// Create a node
	svm_node node;
	node.dim = nbFeatures;
	node.values = const_cast<scalar_t*>(inputSet.features(sample));

	// The predicted labels
	std::vector<double> predictions(nbLabels * (nbLabels - 1) / 2);
	svm_predict_values(model_, &node, &predictions[0]);

	// Update the distribution according to the predictions
	for(unsigned int l0 = 0, l = 0; l0 < nbLabels; ++l0) {
		for(unsigned int l1 = l0 + 1; l1 < nbLabels; ++l1) {
			distr[map_[l0]] += (predictions[l] + 1) / (2 * predictions.size());
			distr[map_[l1]] += (1 - predictions[l]) / (2 * predictions.size());
			++l;
		}
	}
}

void SVM::print(std::ostream& out) const {
	out << "SVM classifier (" << (parameters_.svm_type ? "nu-SVM" : "C-SVM")
		<< ", ";

	switch(parameters_.kernel_type) {
		case LINEAR:
			out << "linear kernel: " << parameters_.gamma << "*u'*v";
			break;
		case POLY:
			out << "polynomial kernel: (" << parameters_.gamma << "*u'*v+"
				<< parameters_.coef0 << ")^" << parameters_.degree;
			break;
		case RBF:
			out << "RBF kernel: exp(-" << parameters_.gamma << "*|u-v|^2)";
			break;
		case SIGMOID:
			out << "sigmoid kernel: tanh(" << parameters_.gamma << "*u'*v+"
				<< parameters_.coef0 << ')';
			break;
		default:
			out << "unknown kernel";
	}

	out << ", " << (parameters_.svm_type ? "nu: " : "C: ")
		<< (parameters_.svm_type ? parameters_.nu : parameters_.C);

	if(!map_.empty()) {
		out << ", labels: ";

		for(unsigned int l = 0; l < map_.size(); ++l) {
			out << (l ? ", " : "") << map_[l];
		}
	}

	out << ")." << std::endl;
}

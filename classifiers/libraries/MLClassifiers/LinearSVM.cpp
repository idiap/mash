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
/// @file MLClassifiers/LinearSVM.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.07.09
/// @version 2.1
//------------------------------------------------------------------------------

#include "LinearSVM.h"

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

//void print_null(const char*) {
	// Do nothing
//}

LinearSVM::LinearSVM(Solver solver,
					 scalar_t cnu)
: model_(0) {
	// Set parameters
	parameters_.solver_type = static_cast<int>(solver);

	// Uses liblinear default values
	parameters_.eps = 0.01;
	parameters_.C = cnu;
	parameters_.nr_weight = 0;
	parameters_.weight_label = 0;
	parameters_.weight = 0;

//	set_print_string_function(&print_null);
}

LinearSVM::~LinearSVM() {
	// Delete the model
	if(model_) {
		free_and_destroy_model(&model_);
	}
}

LinearSVM::LinearSVM(const LinearSVM& svm)
: parameters_(svm.parameters_), model_(0) {
	// Nothing else to copy
}

LinearSVM& LinearSVM::operator=(const LinearSVM& svm) {
	// Delete the model
	if(model_) {
		free_and_destroy_model(&model_);
	}

	// Delete the labels mapping
	map_.clear();

	// Recopy only the parameters
	parameters_ = svm.parameters_;

	return *this;
}

Classifier* LinearSVM::clone() const {
	return new LinearSVM(*this);
}

void LinearSVM::train(InputSet& inputSet) {
	// Sample features from every heuristic in order for the matrix of features
	// to fit in memory
	if(inputSet.nbFeatures() > NB_FEATURES_MAX / inputSet.nbSamples()) {
		inputSet.sampleFeatures(NB_FEATURES_MAX / inputSet.nbSamples(), indices_);
		inputSet.pushFeatures(indices_);
	}
	else {
		indices_.clear();
	}

	// Get the number of features, samples and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Delete the previous model
	if(model_) {
		free_and_destroy_model(&model_);
	}

	// Create a new problem
	problem prob;

	// Recopy the number of samples
	prob.l = nbSamples;
	prob.n = nbFeatures;

	// Recopy the labels
	std::vector<int> labels(inputSet.labels(), inputSet.labels() + nbSamples);
	prob.y = &labels[0];

	// Recopy the features (as we want to normalize them)
	std::vector<scalar_t> matrix;
	inputSet.swapFeatures(matrix);

	// Create samples as expected by liblinear
	std::vector<feature_node> samples(nbSamples);
	prob.x = &samples[0];

	// Compute the mean norm
	scalar_t meanNorm = 0;

	for(unsigned int s = 0; s < nbSamples; ++s) {
		samples[s].dim = nbFeatures;
		samples[s].values = &matrix[s * nbFeatures];

		// Add the mean norm of that sample
		meanNorm += nrm2(samples[s].dim, samples[s].values, 1);
	}

	// Divide the sum of the norms by the number of samples
	meanNorm /= nbSamples;

	std::cout << "[LinearSVM::train] mean(norm): " << meanNorm << '.'
			  << std::endl;

	// Rescale the features so that their mean norm is 1
	std::transform(matrix.begin(), matrix.end(), matrix.begin(),
				   std::bind2nd(std::divides<scalar_t>(), meanNorm));

	// Sets the bias to the default value (liblinear doesn't seem to handle the
	// bias parameter value correctly)
	prob.bias = -1;

	// Make sure that the parameters are correct
	bool crossValidate = parameters_.C < 0;

	// Sets C to a default value in order to pass the parameter check
	if(crossValidate) {
		parameters_.C = 1;
	}

	// There is a problem with the parameters
	assert(!check_parameter(&prob, &parameters_));

	// If C is below zero, use 5-folds cross-validation to determine it
	if(crossValidate) {
		std::vector<int> target(nbSamples); // The predicted labels
		unsigned int nbErrorsMin = nbSamples + 1; // Initialize past the maximum

		for(parameters_.C = 1000; parameters_.C >= 0.01; parameters_.C /= 10) {
			cross_validation(&prob, &parameters_, 5, &target[0]);

			// Count the number of errors
			unsigned int nbErrors = 0;

			for(unsigned int s = 0; s < nbSamples; ++s) {
				if(target[s] != labels[s]) {
					++nbErrors;
				}
			}

			std::cout << "[LinearSVM::train] 5 folds cross-validation error "
						 "for C = " << parameters_.C << ": "
					  << nbErrors * 100.0f / nbSamples << "%." << std::endl;

			// The new C is better than the previous one
			if(nbErrors < nbErrorsMin) {
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
		std::cout << "[LinearSVM::train] optimal C as determined by 5 folds "
			   		 "cross-validation: " << parameters_.C << '.' << std::endl;
	}

	// Train the svm
	model_ = ::train(&prob, &parameters_);
	assert(model_);

	// Reset C so that it will be cross-validated again
	if(crossValidate) {
		parameters_.C = -1;
	}

	// Save libsvm labels
	map_.clear();
	map_.resize(nbLabels);
	get_labels(model_, &map_[0]);

	// Pop the selected features if required
	if(!indices_.empty()) {
		inputSet.popFeatures();
	}
}

void LinearSVM::distribution(InputSet& inputSet,
							 unsigned int sample,
							 scalar_t* distr) const {
	// Push the selected features if required
	if(!indices_.empty()) {
		inputSet.pushFeatures(indices_);
	}

	// Get the number of features and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Make sure that we have a model
	assert(model_);

	// Make sure that there is the same number of labels
	assert(static_cast<unsigned int>(get_nr_class(model_)) <= nbLabels);

	// Create a node
	feature_node node;
	node.dim = nbFeatures;
	node.values = const_cast<scalar_t*>(inputSet.features(sample));

	// The predicted labels
	std::vector<double> predictions(nbLabels);
	predict_values(model_, &node, &predictions[0]);

	// Update the ditribution according to the predictions
	for(unsigned int l = 0; l < nbLabels; ++l) {
		distr[map_[l]] = predictions[l];
	}

	// Pop the selected features if required
	if(!indices_.empty()) {
		inputSet.popFeatures();
	}
}

void LinearSVM::print(std::ostream& out) const {
	out << "LinearSVM classifier (solver: ";

	switch(parameters_.solver_type) {
		case L2R_LR:
			out << "L2-regularized logistic regression";
			break;
		case L2R_L2LOSS_SVC_DUAL:
			out << "L2-regularized L2-loss support vector classification "
				   "(dual)";
			break;
		case L2R_L2LOSS_SVC:
			out << "L2-regularized L2-loss support vector classification "
				   "(primal)";
			break;
		case L2R_L1LOSS_SVC_DUAL:
			out << "L2-regularized L1-loss support vector classification "
				   "(dual)";
			break;
		case MCSVM_CS:
			out << "multi-class support vector classification by Crammer and "
				   "Singer";
			break;
		case L1R_L2LOSS_SVC:
			out << "L1-regularized L2-loss support vector classification";
			break;
		case L1R_LR:
			out << "L1-regularized logistic regression";
			break;
		case L2R_LR_DUAL:
			out << "L2-regularized logistic regression (dual)";
			break;
		default:
			out << "unknown solver";
	}

	out << ", C: " << parameters_.C;

	if(!map_.empty()) {
		out << ", labels: ";

		for(unsigned int l = 0; l < map_.size(); ++l) {
			out << (l ? ", " : "") << map_[l];
		}
	}

	out << ")." << std::endl;

	out << "nr_class: " << model_->nr_class << ", nr_feature: " << model_->nr_feature
		<< ", min(w): " << *std::min_element(model_->w, model_->w + model_->nr_feature)
		<< ", max(w): " << *std::max_element(model_->w, model_->w + model_->nr_feature)
		<< ", labels: " << model_->label[0] << ' ' << model_->label[1] << std::endl;
}

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
/// @file MLClassifiers/C45Tree.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.01.21
/// @version 1.7
//------------------------------------------------------------------------------

#include "C45Tree.h"

#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>

using namespace ML;

C45Tree::C45Tree(scalar_t minWeight,
				 scalar_t confidence,
				 unsigned int maxDepth)
: minWeight_(minWeight), confidence_(confidence), maxDepth_(maxDepth) {
	// The minimum weight of a leaf must be strictly positive
	assert(minWeight > 0);

	// The confidence level must be in the range (0, 1]
	assert(confidence > 0 && confidence <= 1);

	// Set the children to 0
	children_[0] = 0;
	children_[1] = 0;
}

C45Tree::~C45Tree() {
	delete children_[0];
	delete children_[1];
}

C45Tree::C45Tree(const C45Tree& c45tree)
: minWeight_(c45tree.minWeight_), confidence_(c45tree.confidence_),
  maxDepth_(c45tree.maxDepth_), feature_(c45tree.feature_),
  split_(c45tree.split_), label_(c45tree.label_),
  sumWeights_(c45tree.sumWeights_), distr_(c45tree.distr_) {
	// Clone the children (if the tree is not a leaf)
	if(c45tree.children_[0]) {
		children_[0] = static_cast<C45Tree*>(c45tree.children_[0]->clone());
		children_[1] = static_cast<C45Tree*>(c45tree.children_[1]->clone());
	}
	// Else just set them to 0
	else {
		children_[0] = 0;
		children_[1] = 0;
	}
}

C45Tree& C45Tree::operator=(const C45Tree& c45tree) {
	minWeight_ = c45tree.minWeight_;
	confidence_ = c45tree.confidence_;
	maxDepth_ = c45tree.maxDepth_;

	feature_ = c45tree.feature_;
	split_ = c45tree.split_;
	label_ = c45tree.label_;
	sumWeights_ = c45tree.sumWeights_;
	distr_ = c45tree.distr_;

	// Delete the children
	delete children_[0];
	delete children_[1];

	// Clone the children (if the tree is not a leaf)
	if(c45tree.children_[0]) {
		children_[0] = static_cast<C45Tree*>(c45tree.children_[0]->clone());
		children_[1] = static_cast<C45Tree*>(c45tree.children_[1]->clone());
	}
	// Else just set them to 0
	else {
		children_[0] = 0;
		children_[1] = 0;
	}

	return *this;
}

Classifier* C45Tree::clone() const {
	return new C45Tree(*this);
}

void C45Tree::train(InputSet& inputSet) {
	// In case the tree was already trained
	delete children_[0];
	delete children_[1];
	children_[0] = 0;
	children_[1] = 0;

	// Get the number of samples and labels
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Set the maximal depth if needed
	bool maxDepth = false;

	if(!maxDepth_) {
		maxDepth_ = std::ceil(std::log(double(nbLabels)) / std::log(2.0));
		maxDepth = true;
	}

	// Get the labels and the weights of the samples
	const unsigned int* labels = inputSet.labels();
	const scalar_t* weights = inputSet.weights();

	// Make the weights sum to nbSamples as in Dr. Quilans implementation
	std::vector<scalar_t> oldWeights(nbSamples);

	scalar_t norm = std::accumulate(weights, weights + nbSamples, scalar_t());

	std::transform(weights, weights + nbSamples, oldWeights.begin(),
				   std::bind2nd(std::multiplies<scalar_t>(), nbSamples / norm));

	inputSet.swapWeights(oldWeights);

	weights = inputSet.weights();

	// Compute the frequency of appearance of each label
	distr_.clear();
	distr_.resize(nbLabels, 0);

	// Determine the label with the largest frequency
	for(unsigned int s = 0; s < nbSamples; ++s) {
		distr_[labels[s]] += weights[s];
	}

	label_ = std::max_element(distr_.begin(), distr_.end()) - distr_.begin();
	sumWeights_ = nbSamples;

	// Vector of indices over the samples
	std::vector<unsigned int> indices(nbSamples);

	// Make the indices range from o to nbSamples - 1
	for(unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;
	}

	// Create the root (the children will follow recursively)
	make(inputSet, &indices[0], nbSamples);

	if(confidence_ < 1) {
		// Make the indices range from o to nbSamples - 1
		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;
		}

		// Prune the tree previously created
		prune(inputSet, &indices[0], nbSamples, true);
	}

	// Restore the original weights
	inputSet.swapWeights(oldWeights);

	// Restore maxDepth
	if(maxDepth) {
		maxDepth_ = 0;
	}
}

void C45Tree::distribution(InputSet& inputSet,
						   unsigned int sample,
						   scalar_t* distr) const {
	// Get the features of the sample to classifiy
	const scalar_t* features = inputSet.features(sample);

	// The currently selected tree
	const C45Tree* current = this;

	// If the tree has children, find which one is selected
	while(current->children_[0]) {
		assert(current->feature_ < inputSet.nbFeatures());
		if(features[current->feature_] <= current->split_) {
			current = current->children_[0];
		}
		else {
			current = current->children_[1];
		}
	}

	// Recopy the distribution
	assert(current->distr_.size() == inputSet.nbLabels());
	std::copy(current->distr_.begin(), current->distr_.end(), distr);
}

void C45Tree::print(std::ostream& out) const {
	out << "C45Tree classifier (#nodes: " << size() << ", min weight: "
		<< minWeight_ << ", confidence: " << confidence_ << ", max depth: "
		<< maxDepth_ << ')';

	printRec(out, 0);

	out << std::endl;
}

void C45Tree::report(std::vector<unsigned int>& features) const {
	// The tree is not a leaf
	if(children_[0]) {
		features.push_back(feature_);

		// Make the children report too
		children_[0]->report(features);
		children_[1]->report(features);
	}
}

void C45Tree::remap(const std::vector<unsigned int>& mapping) {
	// The tree is not a leaf
	if(children_[0]) {
		feature_ = mapping[feature_];

		// Remaps the children too
		children_[0]->remap(mapping);
		children_[1]->remap(mapping);
	}
}

void C45Tree::make(const InputSet& inputSet,
				   unsigned int* indices,
			 	   unsigned int nbSamples) {
	// Stop if there is only one label (the tree is a leaf), or if we have
	// reached the maximum depth
	assert(distr_.size() > label_);
	if(Utils::geq(distr_[label_], sumWeights_) || !maxDepth_) {
		return;
	}

	// Get the number of features and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Get the labels and the weights of the samples
	const unsigned int* labels = inputSet.labels();
	const scalar_t* weights = inputSet.weights();

	// Compute the node's information from the frequency of each label
	scalar_t information = 0;

	for(unsigned int l = 0; l < nbLabels; ++l) {
		information += Utils::entropy(distr_[l]);
	}

	information -= Utils::entropy(sumWeights_);

	// Current best split in node
	scalar_t largestGain = 0;
	scalar_t sumWeights0 = 0;
	std::vector<scalar_t> distr0;

	// Frequencies of each label before the split. The frequencies of the labels
	// after the split can be directly calculated by subtracting to the total
	// frequencies
	std::vector<double> partialDistr(nbLabels); // Use double's for better
												// precision

	// Try to split on every feature
	for(unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial frequencies to zero
		std::fill_n(partialDistr.begin(), nbLabels, 0);

		// Get the samples (the values of the current feature)
		const scalar_t* samples = inputSet.samples(f);

		// Sort the indices according to the current feature
		Utils::sort(indices, samples, nbSamples);

		// Update the weight of the samples before the split by adding the
		// weight of each sample at every iteration. The weight of the samples
		// after the split is just the sum of the weights minus that weight
		double leftWeight = 0; // Use a double for better precision

		for(unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[s];
			const unsigned int nextIndex = indices[s + 1];
			const unsigned int label = labels[index];
			const scalar_t weight = weights[index];

			partialDistr[label] += weight;
			leftWeight += weight;
			const double rightWeight = sumWeights_ - leftWeight;

			// Make sure the weights of the leaves are sufficient and try only
			// to split in-between two samples with different feature
			if(leftWeight >= minWeight_ && rightWeight >= minWeight_ &&
			   Utils::less(samples[index], samples[nextIndex])) {
				scalar_t leftInfo = 0, rightInfo = 0;

				for(unsigned int l = 0; l < nbLabels; ++l) {
					leftInfo += Utils::entropy(partialDistr[l]);
					rightInfo += Utils::entropy(distr_[l] - partialDistr[l]);
				}

				leftInfo -= Utils::entropy(leftWeight);
				rightInfo -= Utils::entropy(rightWeight);

				// Gain of the split
				scalar_t gain = information - leftInfo - rightInfo;

				// Gain ratio criterion
				//	gain /= Utils::entropy(leftWeight) +
				//			Utils::entropy(rightWeight) -
				//			Utils::entropy(sumWeights_);

				// If it is the best gain so far...
				if(gain > largestGain) {
					largestGain = gain;
					feature_ = f;
					split_ = (samples[index] + samples[nextIndex]) / 2;
					sumWeights0 = leftWeight;
					distr0.assign(partialDistr.begin(), partialDistr.end());
				}
			}
		}
	}

	// If no good split, create a leaf
	if(largestGain < Utils::epsilon) {
		return;
	}

	// Create the two children of the tree
	children_[0] = new C45Tree(minWeight_, confidence_, maxDepth_ - 1);
	children_[1] = new C45Tree(minWeight_, confidence_, maxDepth_ - 1);

	// Assign them the correct label, sumWeights and distribution
	children_[0]->label_ = std::max_element(distr0.begin(), distr0.end()) -
						   distr0.begin();

	// Ok since the distribution is only needed for leaves
	std::transform(distr_.begin(), distr_.end(), distr0.begin(), distr_.begin(),
				   std::minus<scalar_t>());

	children_[1]->label_ = std::max_element(distr_.begin(), distr_.end()) -
						   distr_.begin();

	children_[0]->sumWeights_ = sumWeights0;
	children_[1]->sumWeights_ = sumWeights_ - sumWeights0;

	children_[0]->distr_.swap(distr0);
	children_[1]->distr_.swap(distr_);

	// Build the indices for the left and the right children by partitioning
	// the indices in-place around the pivot split0
	unsigned int rightIndex = Utils::partition(indices,
											   inputSet.samples(feature_),
											   nbSamples,
											   split_);

	// Create the two children recursively
	children_[0]->make(inputSet, indices, rightIndex);
	children_[1]->make(inputSet, indices + rightIndex, nbSamples - rightIndex);
}

/// Used during pruning to calculate the additional error at a leaf.
scalar_t C45Tree::addErrs(scalar_t sumWeights,
				 		  scalar_t error) const {
	const scalar_t Val[] = {0, 0.001, 0.005, 0.01, 0.05, 0.1,  0.2,  0.4,  1};
	const scalar_t Dev[] = {4, 3.09,  2.58,  2.33, 1.65, 1.28, 0.84, 0.25, 0};

	if(error < Utils::epsilon) {
		return sumWeights * (1 - std::exp(std::log(confidence_) / sumWeights));
	}
	else {
		if(error < 1) {
			scalar_t Val0 = sumWeights *
							(1 - std::exp(std::log(confidence_) / sumWeights));

			return Val0 + error * (addErrs(sumWeights, 1) - Val0);
		}
		else if (error + 0.5 >= sumWeights) {
			return 0.67 * (sumWeights - error);
		}
		else {
			unsigned int i = 0;

			while(confidence_ > Val[i]) {
				++i;
			}

			scalar_t Coeff = Dev[i - 1] + (Dev[i] - Dev[i - 1]) *
							 (confidence_ - Val[i - 1]) / (Val[i] - Val[i - 1]);

			Coeff *= Coeff;

			scalar_t Pr = (error + 0.5 + Coeff / 2 +
						   std::sqrt(Coeff * ((error + 0.5) *
											  (1 - (error + 0.5) / sumWeights) +
											  Coeff / 4))) /
						  (sumWeights + Coeff);

			return sumWeights * Pr - error;
		}
	}
}

scalar_t C45Tree::prune(const InputSet& inputSet,
						unsigned int* indices,
						unsigned int nbSamples,
						bool update) {
	// Get the number of labels
	const unsigned int nbLabels = inputSet.nbLabels();

	// Get the labels and the weights of the samples
	const unsigned int* labels = inputSet.labels();
	const scalar_t* weights = inputSet.weights();

	// Compute the frequency of appearance of each label
	std::vector<double> distr(nbLabels, 0); // Use double's for better precision

	// Determine the label with the largest frequency
	for(unsigned int s = 0; s < nbSamples; ++s) {
		distr[labels[indices[s]]] += weights[indices[s]];
	}

	unsigned int label = std::max_element(distr.begin(), distr.end()) -
						 distr.begin();

	double sumWeights = std::accumulate(distr.begin(), distr.end(), 0.0);

	// Update the node if needed
	if(update) {
		label_ = label;
		sumWeights_ = sumWeights;
	}

	// The error if the tree was a leaf
	double leafError = sumWeights - distr[label];

	// Add the uncertainty to obtain an upper bounds on the error
	leafError += addErrs(sumWeights, leafError);

	// If the tree is a leaf
	if(!children_[0]) {
		// Update the node if needed
		if(update) {
			distr_.assign(distr.begin(), distr.end());
		}

		return leafError;
	}
	// If the tree has children first prune them (bottom-up traversal)
	else {
		// Build the indices for the left and the right children by partitioning
		// the indices in-place
		unsigned int rightIndex = Utils::partition(indices,
												   inputSet.samples(feature_),
												   nbSamples,
												   split_);

		// Prune the left child
		scalar_t treeError = children_[0]->prune(inputSet, indices, rightIndex,
												 update);

		// Prune the right child
		treeError += children_[1]->prune(inputSet, indices + rightIndex,
										 nbSamples - rightIndex, update);

		if(!update) {
			return treeError;
		}

		// Compute the classification error on the biggest branch
		C45Tree* biggestBranch = children_[0];
		C45Tree* smallestBranch = children_[1];

		if(biggestBranch->sumWeights_ < smallestBranch->sumWeights_) {
			std::swap(biggestBranch, smallestBranch);
		}

		scalar_t branchError = biggestBranch->prune(inputSet, indices,
													nbSamples, false);

		// The +0.1 constant comes directly from Dr. Quilans implementation
		if(leafError <= branchError + 0.1 && leafError <= treeError + 0.1) {
			// Replace the tree by a leaf
			// Update the node if needed
			distr_.assign(distr.begin(), distr.end());
			delete children_[0];
			delete children_[1];
			children_[0] = 0;
			children_[1] = 0;

			return leafError;
		}
		else if(branchError <= treeError + 0.1) {
			// Replace the tree by it's biggest branch
			delete smallestBranch;

			// Recopy the biggest branch
			feature_ = biggestBranch->feature_;
			split_ = biggestBranch->split_;
			children_[0] = biggestBranch->children_[0];
			children_[1] = biggestBranch->children_[1];

			// Delete the biggest branch top node
			biggestBranch->children_[0] = 0;
			biggestBranch->children_[1] = 0;
			delete biggestBranch;

			// Update the subtrees
			prune(inputSet, indices, nbSamples, true);

			return branchError;
		}
		else {
			return treeError;
		}
	}
}

void C45Tree::printRec(std::ostream& out,
			  		   unsigned int depth) const {
	// If the tree is a leaf
	if(!children_[0]) {
		assert(distr_.size() > label_);
		out << ' ' << label_ << " (" << sumWeights_ << '/'
			<< (sumWeights_ - distr_[label_]) << ')';
	}
	else {
		out << std::endl;

		// Output as many '|' as needed
		for(unsigned int d = 0; d < depth; ++d) {
			out << '|' << '\t';
		}

		out << "feature " << feature_ << " <= " << split_ << ':';

		children_[0]->printRec(out, depth + 1);

		out << std::endl;

		// Output as many '|' as needed
		for(unsigned int d = 0; d < depth; ++d) {
			out << '|' << '\t';
		}

		out << "feature " << feature_ << " > " << split_ << ':';

		children_[1]->printRec(out, depth + 1);
	}
}

unsigned int C45Tree::size() const {
	// The tree is not a leaf
	if(children_[0]) {
		return 1 + children_[0]->size() + children_[1]->size();
	}
	else {
		return 0;
	}
}

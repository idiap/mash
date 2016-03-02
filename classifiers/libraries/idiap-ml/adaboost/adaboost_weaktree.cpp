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


/// \file	adaboost_weaktree.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 28, 2011

#include "adaboost_weaktree.h"

#include <idiap-ml/dataset/dataset_subset.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace ml;

WeakTree::WeakTree(unsigned int maxDepth) : maxDepth_(maxDepth) {}
#if 0
void WeakTree::train(const IDataSet& dataset,
					 const std::vector<std::vector<double> >& weights) {
	// Get the number of samples and labels
	const unsigned int nbSamples = dataset.nbSamples();
	const unsigned int nbLabels = dataset.nbLabels();

	// Make sure that there is enough weights
	if (weights.size() != nbSamples) {
		throw std::logic_error("the number of weights is different from the "
							   "number of labels");
	}

	// Clear the tree if it was previously trained
	nodes_.clear();

	// Vector of pointer to the weights
	std::vector<const double*> weightPointers(nbSamples);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		weightPointers[s] = &weights[s][0];
	}

	// Default depth of the tree
	unsigned int depth = maxDepth_;

	if (!maxDepth_) {
		depth = std::ceil(std::log(double(nbLabels)) / std::log(2.0));
	}

	train(dataset, &weightPointers[0], depth);
}
#else
void WeakTree::train(const IDataSet& dataset,
					 const std::vector<std::vector<double> >& weights) {
	// Get the number of samples, features, and labels
	const unsigned int nbSamples = dataset.nbSamples();
	const unsigned int nbFeatures = dataset.nbFeatures();
	const unsigned int nbLabels = dataset.nbLabels();

	// Make sure that there is enough weights
	if (weights.size() != nbSamples) {
		throw std::logic_error("the number of weights is different from the "
							   "number of labels");
	}

	// Clear the tree if it was previously trained
	nodes_.clear();

	// Compute the whole matrix of features
	std::vector<float> values(nbSamples * nbFeatures);

	std::vector<unsigned int> indices(std::max(nbSamples, nbFeatures));

	for (unsigned int i = 0; i < indices.size(); ++i) {
		indices[i] = i;
	}

	dataset.computeFeatures(nbSamples, &indices[0], nbFeatures, &indices[0],
							&values[0], true);

	// Vector of pointers to the samples
	std::vector<const float*> samplePointers(nbFeatures);

	for (unsigned int f = 0; f < nbFeatures; ++f) {
		samplePointers[f] = &values[f * nbSamples];
	}

	// Vector of pointer to the weights
	std::vector<const double*> weightPointers(nbSamples);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		weightPointers[s] = &weights[s][0];
	}

	// Default depth of the tree
	unsigned int depth = maxDepth_;

	if (!maxDepth_) {
		depth = std::ceil(std::log(double(nbLabels)) / std::log(2.0));
	}

	train(&samplePointers[0], &weightPointers[0], &indices[0], nbSamples,
		  nbFeatures, nbLabels, depth);
}
#endif
unsigned int WeakTree::classify(const IDataSet& dataset,
								unsigned int sample) const {
	// Make sure the tree was already trained
	if (nodes_.empty()) {
		throw std::logic_error("the weak classifier was not trained");
	}

	// The node currently selected
	int current = 0;

	// If the node has children, find which one is selected
	do {
		float value;
		dataset.computeFeatures(1, &sample, 1, &nodes_[-current].feature_,
								&value);

		current = nodes_[-current].labels_[value >= nodes_[-current].split_];

#ifndef NDEBUG
		if (-current >= static_cast<int>(nodes_.size())) {
			throw std::out_of_range("the current node is not part of the tree");
		}
#endif
	} while (current < 0);

	return current;
}

void WeakTree::report(std::vector<unsigned int>& features) const {
	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		features.push_back(nodes_[n].feature_);
	}
}

void WeakTree::load(const json::Value& value) {
	if (value["name"].string() != "WeakTree") {
		throw std::ios::failure("parse error");
	}

	maxDepth_ = value["maxDepth"];

	const json::Value& nodes(value["nodes"]);
	nodes_.resize(nodes.array().size());

	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		const json::Value& node(nodes[n]);
		nodes_[n].feature_ = node["feature"];
		nodes_[n].split_ = node["split"];
		nodes_[n].labels_[0] = node["labels"].array()[0];
		nodes_[n].labels_[1] = node["labels"].array()[1];
	}
}

void WeakTree::save(json::Value& value) const {
	value["name"] = "WeakTree";
	value["maxDepth"] = maxDepth_;

	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		json::Value node;
		node["feature"] = nodes_[n].feature_;
		node["split"] = nodes_[n].split_;
		node["labels"].array().assign(nodes_[n].labels_, nodes_[n].labels_ + 2);
		value["nodes"].array().push_back(node);
	}
}

void WeakTree::print(std::ostream& out) const {
	out << "WeakTree classifier (#nodes: " << nodes_.size() << ", max depth: "
		<< maxDepth_ << ')';

	print(out, 0, 0);

	out << std::endl;
}

void WeakTree::remap(const std::vector<unsigned int>& mapping) {
	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		nodes_[n].feature_ = mapping[nodes_[n].feature_];
	}
}

class indicesComparator {
public:
	indicesComparator(const float* values) : values_(values) {}

	bool operator()(unsigned int a,
					unsigned int b) const {
		return values_[a] < values_[b];
	}

private:
	const float* const values_; ///< The scalar_t vector of values
};

class indexLessThanPivot {
public:
	indexLessThanPivot(const float* values,
					   float pivot) : values_(values), pivot_(pivot) {}

	bool operator()(unsigned int a) const {
		return values_[a] < pivot_;
	}

private:
	const float* const values_; ///< The vector of values
	const float pivot_;         ///< The pivot
};
#if 0
bool WeakTree::train(const IDataSet& dataset,
					 const double** weights,
					 unsigned int depth) {
	// Get the number of samples, features, and labels
	const unsigned int nbSamples = dataset.nbSamples();
	const unsigned int nbFeatures = dataset.nbFeatures();
	const unsigned int nbLabels = dataset.nbLabels();

	// Compute the sums of the columns of the weights matrix
	std::vector<double> sums(nbLabels);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		for (unsigned int l = 0; l < nbLabels; ++l) {
			sums[l] += weights[s][l];
		}
	}

	// The currently predicted label is the one with minimum weight
	unsigned int label = static_cast<unsigned int>(
		std::min_element(sums.begin(), sums.end()) - sums.begin());

	// Create a node to represent the current subtree
	Node node;
	node.feature_ = 0;
	node.split_ = 0.0;
	node.labels_[0] = label;
	node.labels_[1] = label;

	// Weight of the node so far
	double bestWeight = sums[label];

	// Sums of the columns of the weights matrix before the split
	std::vector<double> partialSums(nbLabels);

	// The values of the current feature
	std::vector<float> samples(nbSamples);

	// Vector of indices over the samples
	std::vector<unsigned int> indices(nbSamples);

	// Try to split on every feature
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial sums to zero
		std::fill(partialSums.begin(), partialSums.end(), 0.0);

		// Make the indices range from 0 to nbSamples
		for (unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;
		}

		// Get the samples (the values of the current feature)
		dataset.computeFeatures(nbSamples, &indices[0], 1, &f, &samples[0], true);

		// Sort the indices according to the current feature
		std::sort(indices.begin(), indices.end(),
				  indicesComparator(&samples[0]));

		// Update the weight of the samples before the split by adding the
		// weight of each sample at every iteration. The weight of the samples
		// after the split is just the sum of the weights minus that weight
		for (unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[s];
			const unsigned int nextIndex = indices[s + 1];

			for (unsigned int l = 0; l < nbLabels; ++l) {
				partialSums[l] += weights[index][l];
			}

			// Try only to split in-between two samples with different feature
			if (samples[index] < samples[nextIndex]) {
				double leftWeight = partialSums[0];
				double rightWeight = sums[0] - partialSums[0];
				unsigned int leftLabel = 0;
				unsigned int rightLabel = 0;

				for (unsigned int l = 1; l < nbLabels; ++l) {
					if (partialSums[l] < leftWeight) {
						leftWeight = partialSums[l];
						leftLabel = l;
					}
					else if (sums[l] - partialSums[l] < rightWeight) {
						rightWeight = sums[l] - partialSums[l];
						rightLabel = l;
					}
				}

				double weight = leftWeight + rightWeight;

				if (weight < bestWeight) {
					node.feature_ = f;
					node.split_ = (samples[index] + samples[nextIndex]) * 0.5f;
					node.labels_[0] = leftLabel;
					node.labels_[1] = rightLabel;
					bestWeight = weight;
				}
			}
		}
	}

	// If the node is constant then there is nothing to do
	if (bestWeight == sums[label] && !nodes_.empty()) {
		return false;
	}

	// Add the node
	const unsigned int index = static_cast<unsigned int>(nodes_.size());
	nodes_.push_back(node);

	// If the maximum depth was reached then there is nothing to do
	if (depth <= 1) {
		return true;
	}

	// Make the indices range from 0 to nbSamples
	for (unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;
	}

	// Get the samples (the values of the current feature)
	dataset.computeFeatures(nbSamples, &indices[0], 1, &node.feature_,
							&samples[0], true);

	// Build the indices for the left and the right children by partitioning
	// the indices in-place around the pivot
	unsigned int rightIndex = static_cast<unsigned int>(
		std::partition(indices.begin(), indices.end(),
					   indexLessThanPivot(&samples[0], node.split_)) -
		indices.begin());

	// Partition the weights matrix
	std::vector<const double*> leftWeights(rightIndex);
	std::vector<const double*> rightWeights(nbSamples - rightIndex);

	for (unsigned int s = 0; s < rightIndex; ++s) {
		leftWeights[s] = weights[indices[s]];
	}

	for (unsigned int s = rightIndex; s < nbSamples; ++s) {
		rightWeights[s - rightIndex] = weights[indices[s]];
	}

	// Train the first child
	int child = static_cast<int>(nodes_.size());

	if (train(SampleSubSet(dataset, rightIndex, &indices[0]),
			  &leftWeights[0], depth - 1)) {
		nodes_[index].labels_[0] = -child;
	}

	// Train the second child
	child = static_cast<int>(nodes_.size());

	if (train(SampleSubSet(dataset, nbSamples - rightIndex,
						   &indices[rightIndex]),
			  &rightWeights[0], depth - 1)) {
		nodes_[index].labels_[1] = -child;
	}

	return true;
}
#else
bool WeakTree::train(const float** samples,
					 const double** weights,
					 unsigned int* indices,
					 unsigned int nbSamples,
					 unsigned int nbFeatures,
					 unsigned int nbLabels,
					 unsigned int depth) {
	// Compute the sums of the columns of the weights matrix
	std::vector<double> sums(nbLabels);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		for (unsigned int l = 0; l < nbLabels; ++l) {
			sums[l] += weights[indices[s]][l];
		}
	}

	// The currently predicted label is the one with minimum weight
	unsigned int label = static_cast<unsigned int>(
		std::min_element(sums.begin(), sums.end()) - sums.begin());

	// Create a node to represent the current subtree
	Node node;
	node.feature_ = 0;
	node.split_ = 0.0;
	node.labels_[0] = label;
	node.labels_[1] = label;

	// Weight of the node so far
	double bestWeight = sums[label];

	// Sums of the columns of the weights matrix before the split
	std::vector<double> partialSums(nbLabels);

	// Try to split on every feature
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial sums to zero
		std::fill(partialSums.begin(), partialSums.end(), 0.0);

		// Sort the indices according to the current feature
		std::sort(indices, indices + nbSamples,
				  indicesComparator(samples[f]));

		// Update the weight of the samples before the split by adding the
		// weight of each sample at every iteration. The weight of the samples
		// after the split is just the sum of the weights minus that weight
		for (unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[s];
			const unsigned int nextIndex = indices[s + 1];

			for (unsigned int l = 0; l < nbLabels; ++l) {
				partialSums[l] += weights[index][l];
			}

			// Try only to split in-between two samples with different feature
			if (samples[f][index] < samples[f][nextIndex]) {
				double leftWeight = partialSums[0];
				double rightWeight = sums[0] - partialSums[0];
				unsigned int leftLabel = 0;
				unsigned int rightLabel = 0;

				for (unsigned int l = 1; l < nbLabels; ++l) {
					if (partialSums[l] < leftWeight) {
						leftWeight = partialSums[l];
						leftLabel = l;
					}
					else if (sums[l] - partialSums[l] < rightWeight) {
						rightWeight = sums[l] - partialSums[l];
						rightLabel = l;
					}
				}

				double weight = leftWeight + rightWeight;

				if (weight < bestWeight) {
					node.feature_ = f;
					node.split_ = (samples[f][index] + samples[f][nextIndex]) * 0.5f;
					node.labels_[0] = leftLabel;
					node.labels_[1] = rightLabel;
					bestWeight = weight;
				}
			}
		}
	}

	// If the node is constant then there is nothing to do
	if (bestWeight == sums[label] && !nodes_.empty()) {
		return false;
	}

	// Add the node
	const unsigned int index = static_cast<unsigned int>(nodes_.size());
	nodes_.push_back(node);

	// If the maximum depth was reached then there is nothing to do
	if (depth <= 1) {
		return true;
	}

	// Build the indices for the left and the right children by partitioning
	// the indices in-place around the pivot
	unsigned int rightIndex = static_cast<unsigned int>(
		std::partition(indices, indices + nbSamples,
					   indexLessThanPivot(samples[node.feature_], node.split_)) -
		indices);

	// Train the first child
	int child = static_cast<int>(nodes_.size());

	if (train(samples, weights, indices, rightIndex, nbFeatures, nbLabels,
			  depth - 1)) {
		nodes_[index].labels_[0] = -child;
	}

	// Train the second child
	child = static_cast<int>(nodes_.size());

	if (train(samples, weights, indices + rightIndex, nbSamples - rightIndex,
			  nbFeatures, nbLabels, depth - 1)) {
		nodes_[index].labels_[1] = -child;
	}

	return true;
}
#endif
void WeakTree::print(std::ostream& out,
					 int index,
					 unsigned int depth) const {
	out << std::endl;

	// Output as many '|' as needed
	for (unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << "  < "
		<< nodes_[index].split_ << ':';

	if (nodes_[index].labels_[0] >= 0) {
		out << ' ' << nodes_[index].labels_[0];
	}
	else {
		print(out,-nodes_[index].labels_[0], depth + 1);
	}

	out << std::endl;

	// Output as many '|' as needed
	for (unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << " >= "
		<< nodes_[index].split_ << ':';

	if (nodes_[index].labels_[1] >= 0) {
		out << ' ' << nodes_[index].labels_[1];
	}
	else {
		print(out,-nodes_[index].labels_[1], depth + 1);
	}
}

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


/// \file	adaboost_mh_weaktree.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 4, 2011

#include "adaboost_mh_weaktree.h"

#include <idiap-ml/dataset/dataset_subset.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace ml;

WeakTreeMH::WeakTreeMH(unsigned int maxDepth) : maxDepth_(maxDepth) {
	// Nothing to do
}
#if 0
void WeakTreeMH::train(const IDataSet& dataset,
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
void WeakTreeMH::train(const IDataSet& dataset,
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

	// Vector of labels
	std::vector<unsigned int> labels(nbSamples);

	// Vector of pointer to the weights
	std::vector<const double*> weightPointers(nbSamples);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		labels[s] = dataset.label(s);
		weightPointers[s] = &weights[s][0];
	}

	// Default depth of the tree
	unsigned int depth = maxDepth_;

	if (!maxDepth_) {
		depth = std::ceil(std::log(double(nbLabels)) / std::log(2.0));
	}

	train(&samplePointers[0], &labels[0], &weightPointers[0], &indices[0],
		  nbSamples, nbFeatures, nbLabels, depth);
}
#endif
void WeakTreeMH::distribution(const IDataSet& dataset,
							  unsigned int sample,
							  char* distr) const {
	// Make sure the tree was already trained
	if (nodes_.empty()) {
		throw std::logic_error("the weak classifier was not trained");
	}

	// The node currently selected
	unsigned int current = 0;

	float value;

	while (nodes_[current].children_[0]) {
		dataset.computeFeatures(1, &sample, 1, &nodes_[current].feature_,
								&value);

		current = nodes_[current].children_[value >= nodes_[current].split_];

#ifndef NDEBUG
		if (current >= nodes_.size()) {
			throw std::out_of_range("the current node is not part of the tree");
		}
#endif
	}

	dataset.computeFeatures(1, &sample, 1, &nodes_[current].feature_,
							&value);

	bool phi = value >= nodes_[current].split_;

	// Get the number of labels
	const unsigned int nbLabels = std::min(dataset.nbLabels(),
	   static_cast<unsigned int>(nodes_[current].signs_.size()));

	for (unsigned int l = 0; l < nbLabels; ++l) {
		distr[l] = (phi == nodes_[current].signs_[l]) ? 1 : -1;
	}
}

void WeakTreeMH::report(std::vector<unsigned int>& features) const {
	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		features.push_back(nodes_[n].feature_);
	}
}

void WeakTreeMH::load(const json::Value& value) {
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
		nodes_[n].signs_.assign(node["signs"].array().begin(), node["signs"].array().end());
		nodes_[n].children_[0] = node["children"].array()[0];
		nodes_[n].children_[1] = node["children"].array()[1];
	}
}

void WeakTreeMH::save(json::Value& value) const {
	value["name"] = "WeakTree";
	value["maxDepth"] = maxDepth_;

	for (unsigned int n = 0; n < nodes_.size(); ++n) {
		json::Value node;
		node["feature"] = nodes_[n].feature_;
		node["split"] = nodes_[n].split_;
		node["signs"].array().assign(nodes_[n].signs_.begin(), nodes_[n].signs_.end());
		node["children"].array().assign(nodes_[n].children_, nodes_[n].children_ + 2);
		value["nodes"].array().push_back(node);
	}
}

void WeakTreeMH::print(std::ostream& out) const {
	out << "WeakTree classifier (#nodes: " << nodes_.size()
		<< ", max depth: " << maxDepth_ << ')';

	print(out, 0, 0);

	out << std::endl;
}

void WeakTreeMH::remap(const std::vector<unsigned int>& mapping) {
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
void WeakTreeMH::train(const IDataSet& dataset,
					   const double** weights,
					   unsigned int depth) {
	// Get the number of samples, features, and labels
	const unsigned int nbSamples = dataset.nbSamples();
	const unsigned int nbFeatures = dataset.nbFeatures();
	const unsigned int nbLabels = dataset.nbLabels();

	// Compute the edges of the labels
	std::vector<double> edges(nbLabels);

	for (unsigned int l = 0; l < nbLabels; ++l) {
		for (unsigned int s = 0; s < nbSamples; ++s) {
			edges[l] += (dataset.label(s) == l) ? weights[s][l] :
												 -weights[s][l];
		}
	}

	// Create a node to represent the current subtree
	Node node;
	node.feature_ = 0;
	node.split_ = -std::numeric_limits<float>::max();
	node.signs_.resize(nbLabels, false);
	node.children_[0] = node.children_[1] = 0;

	// The best sum of absolute values of edges so far
	double sumEdges = 0.0;

	for (unsigned int l = 0; l < nbLabels; ++l) {
		node.signs_[l] = (edges[l] >= 0.0);
		sumEdges += std::abs(edges[l]);
	}

	// Edges before the split
	std::vector<double> partialEdges(nbLabels);

	// The values of the current feature
	std::vector<float> samples(nbSamples);

	// Vector of indices over the samples
	std::vector<unsigned int> indices(nbSamples);

	// Try to split on every feature
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial sums to zero
		std::fill(partialEdges.begin(), partialEdges.end(), 0.0);

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

			// Include the current sample in the partial edges
			double sum = 0.0;

			for(unsigned int l = 0; l < nbLabels; ++l) {
				partialEdges[l] += (dataset.label(index) == l) ? weights[index][l] :
																-weights[index][l];

				sum += std::abs(edges[l] - 2.0 * partialEdges[l]);
			}

			// Try only to split in-between two samples with different feature
			if (samples[index] < samples[nextIndex] && sum > sumEdges) {
				node.feature_ = f;
				node.split_ = (samples[index] + samples[nextIndex]) * 0.5f;

				for (unsigned int l = 0; l < nbLabels; ++l) {
					node.signs_[l] = edges[l] >= 2.0 * partialEdges[l];
				}

				sumEdges = sum;
			}
		}
	}

	// Add the node
	const unsigned int index = static_cast<unsigned int>(nodes_.size());
	nodes_.push_back(node);

	// If the maximum depth was reached then there is nothing to do
	if (node.split_ == -std::numeric_limits<float>::max() || depth <= 1) {
		return;
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

	// Partition the weight matrix
	std::vector<const double*> leftWeights(rightIndex);
	std::vector<const double*> rightWeights(nbSamples - rightIndex);

	for (unsigned int s = 0; s < rightIndex; ++s) {
		leftWeights[s] = weights[indices[s]];
	}

	for (unsigned int s = rightIndex; s < nbSamples; ++s) {
		rightWeights[s - rightIndex] = weights[indices[s]];
	}

	// Train the first child
	nodes_[index].children_[0] = static_cast<unsigned int>(nodes_.size());

	train(SampleSubSet(dataset, rightIndex, &indices[0]),
		  &leftWeights[0], depth - 1);

	// Train the second child
	nodes_[index].children_[1] = static_cast<unsigned int>(nodes_.size());

	train(SampleSubSet(dataset, nbSamples - rightIndex, &indices[rightIndex]),
		  &rightWeights[0], depth - 1);
}
#else
void WeakTreeMH::train(const float** samples,
					   const unsigned int* labels,
					   const double** weights,
					   unsigned int* indices,
					   unsigned int nbSamples,
					   unsigned int nbFeatures,
					   unsigned int nbLabels,
					   unsigned int depth) {
	// Compute the edges of the labels
	std::vector<double> edges(nbLabels);

	for (unsigned int l = 0; l < nbLabels; ++l) {
		for (unsigned int s = 0; s < nbSamples; ++s) {
			unsigned int index = indices[s];
			edges[l] += (labels[index] == l) ? weights[index][l] :
											  -weights[index][l];
		}
	}

	// Create a node to represent the current subtree
	Node node;
	node.feature_ = 0;
	node.split_ = -std::numeric_limits<float>::max();
	node.signs_.resize(nbLabels, false);
	node.children_[0] = node.children_[1] = 0;

	// The best sum of absolute values of edges so far
	double sumEdges = 0.0;

	for (unsigned int l = 0; l < nbLabels; ++l) {
		node.signs_[l] = (edges[l] >= 0.0);
		sumEdges += std::abs(edges[l]);
	}

	// Edges before the split
	std::vector<double> partialEdges(nbLabels);

	// Try to split on every feature
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial sums to zero
		std::fill(partialEdges.begin(), partialEdges.end(), 0.0);

		// Sort the indices according to the current feature
		std::sort(indices, indices + nbSamples,
				  indicesComparator(samples[f]));

		// Update the weight of the samples before the split by adding the
		// weight of each sample at every iteration. The weight of the samples
		// after the split is just the sum of the weights minus that weight
		for (unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[s];
			const unsigned int nextIndex = indices[s + 1];

			// Include the current sample in the partial edges
			double sum = 0.0;

			for(unsigned int l = 0; l < nbLabels; ++l) {
				partialEdges[l] += (labels[index] == l) ? weights[index][l] :
														 -weights[index][l];

				sum += std::abs(edges[l] - 2.0 * partialEdges[l]);
			}

			// Try only to split in-between two samples with different feature
			if (samples[f][index] < samples[f][nextIndex] && sum > sumEdges) {
				node.feature_ = f;
				node.split_ = (samples[f][index] + samples[f][nextIndex]) * 0.5f;

				for (unsigned int l = 0; l < nbLabels; ++l) {
					node.signs_[l] = edges[l] >= 2.0 * partialEdges[l];
				}

				sumEdges = sum;
			}
		}
	}

	// Add the node
	const unsigned int index = static_cast<unsigned int>(nodes_.size());
	nodes_.push_back(node);

	// If the maximum depth was reached then there is nothing to do
	if (node.split_ == -std::numeric_limits<float>::max() || depth <= 1) {
		return;
	}

	// Build the indices for the left and the right children by partitioning
	// the indices in-place around the pivot
	unsigned int rightIndex = static_cast<unsigned int>(
		std::partition(indices, indices + nbSamples,
					   indexLessThanPivot(samples[node.feature_], node.split_)) -
		indices);

	// Train the first child
	nodes_[index].children_[0] = static_cast<unsigned int>(nodes_.size());

	train(samples, labels, weights, indices, rightIndex, nbFeatures, nbLabels,
		  depth - 1);

	// Train the second child
	nodes_[index].children_[1] = static_cast<unsigned int>(nodes_.size());

	train(samples, labels, weights, indices + rightIndex,
		  nbSamples - rightIndex, nbFeatures, nbLabels, depth - 1);
}
#endif

void WeakTreeMH::print(std::ostream& out,
					 int index,
					 unsigned int depth) const {
	out << std::endl;

	// Output as many '|' as needed
	for (unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << "  < "
		<< nodes_[index].split_ << ':';

	if (!nodes_[index].children_[0]) {
		out << ' ';

		for (unsigned int l = 0; l < nodes_[index].signs_.size(); ++l) {
			out << (nodes_[index].signs_[l] ? '-' : '+');
		}
	}
	else {
		print(out, nodes_[index].children_[0], depth + 1);
	}

	out << std::endl;

	// Output as many '|' as needed
	for (unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << " >= "
		<< nodes_[index].split_ << ':';

	if (!nodes_[index].children_[1]) {
		out << ' ';

		for (unsigned int l = 0; l < nodes_[index].signs_.size(); ++l) {
			out << (nodes_[index].signs_[l] ? '+' : '-');
		}
	}
	else {
		print(out, nodes_[index].children_[1], depth + 1);
	}
}

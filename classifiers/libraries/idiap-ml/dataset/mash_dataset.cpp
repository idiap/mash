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


/// \file	mash_dataset.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 21, 2011

#include "mash_dataset.h"

#include <idiap-ml/utils/transpose.h>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

using namespace ml;

MashDataSet::MashDataSet(Mash::IClassifierInputSet* mashIClassifierInputSet, unsigned int maxNegatives): inputset_(mashIClassifierInputSet), maxNegatives_(maxNegatives)
{
	// There must be at least one label and heuristic

	nbLabels_ = inputset_->nbLabels();
	unsigned int nbImages = inputset_->nbImages();
	unsigned int nbHeuristics = inputset_->nbHeuristics();

	if (nbLabels_ < 2 || !nbHeuristics)
	{
		throw std::invalid_argument("there must be at least 2 labels and 1 " "heuristic");
	}

	// Save the label, image, and coordinates of every target object in every
	// image
	unsigned int nbSamples = 0;
	unsigned int nbNegatives = 0;

	// For every image
	for (unsigned int i = 0; i < nbImages; ++i)
	{
		// For every object in the image
		Mash::tObjectsList objects;
		inputset_->objectsInImage(i, &objects);

		for (Mash::tObjectsIterator obj = objects.begin(), objEnd = objects.end(); obj != objEnd; ++obj)
		{
			if (obj->target)
			{
				labels_.push_back(obj->label);
				images_.push_back(i);
				coordinates_.push_back(obj->roi_position);
				++nbSamples;
			}
		}

		// Sample negatives from every image
		if (inputset_->isDoingDetection() && maxNegatives_)
		{
			Mash::tCoordinatesList negatives;
			inputset_->negativesInImage(i, &negatives);

			// Sample at most maxNegatives positions
			unsigned int step = std::max(static_cast<unsigned int>(negatives.size()) /  maxNegatives_, 1U);

			unsigned int n = 0;

			for (Mash::tCoordinatesIterator coord = negatives.begin() + step / 2, coordEnd = negatives.end(); coord != coordEnd && n < maxNegatives; coord += step)
			{
				labels_.push_back(nbLabels_);
				images_.push_back(i);
				coordinates_.push_back(*coord);
				++n;
			}

			nbNegatives += n;
		}
	}

	// Compute the total number of features
	unsigned int nbFeatures = 0;

	// For every feature of every heuristic
	for (unsigned int h = 0; h < nbHeuristics; ++h) {
		heuristics_.push_back(nbFeatures += inputset_->nbFeatures(h));
	}

	// There must be at least one feature
	if (!nbFeatures)
	{
		throw std::invalid_argument("there must be at least 1 feature");
	}

	// Print some info about the input set to the log
	std::cout << "[MashDataSet::MashDataSet] Created dataset, #samples: " << nbSamples;

	if (nbNegatives)
	{
		std::cout << " + " << nbNegatives;
	}

	std::cout << ", #features: " << nbFeatures << ", #labels: " << nbLabels_
			  << (nbNegatives ? " + 1" : "") << ", #images: " << nbImages
			  << ", #heuristics: " << nbHeuristics << std::endl;

	// Add one label for the negatives
	if (nbNegatives)
	{
		++nbLabels_;
	}
}

unsigned int MashDataSet::nbSamples() const {
	return labels_.size();
}

unsigned int MashDataSet::nbFeatures() const {
	return heuristics_.back();
}

unsigned int MashDataSet::nbLabels() const {
	return nbLabels_;
}

unsigned int MashDataSet::nbHeuristics() const {
	return heuristics_.size();
}

void MashDataSet::computeFeatures(unsigned int nbSamples,
								  const unsigned int* sampleIndices,
								  unsigned int nbFeatures,
								  const unsigned int* featureIndices,
								  float* values,
								  bool trans) const {
	// Make sure that the indices are not 0
	if (!sampleIndices || !featureIndices) {
		throw std::invalid_argument("no sample or feature indices provided");
	}

	// The number of heuristics
	const unsigned int nbHeuristics = heuristics_.size();

	// Partition the feature indices by heuristic
	typedef std::pair<unsigned int, unsigned int> Pair;

	std::vector<Pair> indices(nbFeatures);

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		indices[f] = Pair(featureIndices[f], f);
	}

	// Indices to give to the computeSomeFeature function
	std::vector<unsigned int> indexes(nbFeatures);

	// Number of features selected by heuristic
	std::vector<unsigned int> selected(nbHeuristics);

	// Iterator to the first index
	std::vector<Pair>::iterator first = indices.begin();

	// For every heuristic
	for (unsigned int h = 0, j = 0; h < nbHeuristics; ++h) {
		// Select only the features of the heuristic h
		std::vector<Pair>::iterator middle =
			std::partition(first, indices.end(),
						   std::bind2nd(std::less<Pair>(),
										Pair(heuristics_[h], 0)));

		// Save the number of features selected by the heuristic
		selected[h] = middle - first;

		// Subtract the first index of the heuristic
		for (unsigned int i = 0; i < selected[h]; ++i) {
			indexes[j + i] = first[i].first - (h ? heuristics_[h - 1] : 0);
		}

		j += selected[h];

		// Select the next partition
		first = middle;
	}

	// Temporary buffer to hold the computed features
	std::vector<Mash::scalar_t> temp(*std::max_element(selected.begin(),
													   selected.end()));

	// Report to the log the progress in filling the cache if there is more
	// than one sample and one feature
	if (nbSamples > 1 && nbFeatures > 1) {
		std::cout << "[MashDataSet::computeFeatures] Computing " << nbSamples
				  << 'x' << nbFeatures << " features...";
	}

	unsigned int percentage = 100;

	// For all the selected samples
	for (unsigned int s = 0; s < nbSamples; ++s) {
		// Recopy the computed features at the proper place in the cache
		for (unsigned int h = 0, j = 0; h < nbHeuristics; ++h) {
			if (selected[h]) {
				// computeSomeFeatures doesn't like to compute 0 features
#ifndef NDEBUG
				if (!inputset_->computeSomeFeatures(images_.at(sampleIndices[s]),
													coordinates_.at(sampleIndices[s]),
													h, selected[h], &indexes[j],
													&temp[0])) {
#else
				if (!inputset_->computeSomeFeatures(images_[sampleIndices[s]],
													coordinates_[sampleIndices[s]],
													h, selected[h], &indexes[j],
													&temp[0])) {
#endif
					throw std::runtime_error("computeSomeFeatures returned false");
				}

				for (unsigned int k = 0; k < selected[h]; ++k) {
					values[s * nbFeatures + indices[j + k].second] = temp[k];
				}

				j += selected[h];
			}
		}

		unsigned int percent = s * 100 / nbSamples;

		if (nbSamples > 1 && nbFeatures > 1 && percent != percentage) {
			std::cout << ' ' << (percentage = percent) << '%' << std::flush;
		}
	}

	if (trans) {
		transpose(values, nbSamples, nbFeatures);
	}

	if (percentage != 100) {
		std::cout << " 100%" << std::endl;
	}
}

unsigned int MashDataSet::label(unsigned int sample) const {
#ifndef NDEBUG
	return labels_.at(sample);
#else
	return labels_[sample];
#endif
}

unsigned int MashDataSet::heuristic(unsigned int feature) const {
#ifndef NDEBUG
	if (feature >= nbFeatures()) {
		throw std::out_of_range("the feature index is out of range");
	}
#endif
	// Find the heuristic into which f falls
	return std::upper_bound(heuristics_.begin(), heuristics_.end(), feature) -
		   heuristics_.begin();
}

void MashDataSet::pushSample(unsigned int label,
							 unsigned int image,
							 Mash::coordinates_t position) {
	labels_.push_back(label);
	images_.push_back(image);
	coordinates_.push_back(position);
}

void MashDataSet::popSample() {
	if (labels_.empty()) {
		throw std::length_error("stack underflow");
	}

	labels_.pop_back();
	images_.pop_back();
	coordinates_.pop_back();
}

void MashDataSet::convert(const std::vector<unsigned int>& features, Mash::tFeatureList& list) const {
	for (unsigned int f = 0; f < features.size(); ++f) {
		unsigned int h = heuristic(features[f]);
		list.push_back(Mash::tFeature(h, features[f] - (h ? heuristics_[h - 1] : 0)));
	}
}

void MashDataSet::convert(const Mash::tFeatureList& list,
						  std::vector<unsigned int>& features) const {
	for (unsigned int f = 0; f < list.size(); ++f) {
		unsigned int h = list[f].heuristic ? heuristics_.at(list[f].heuristic - 1) : 0;
		features.push_back(h + list[f].feature_index);
	}
}

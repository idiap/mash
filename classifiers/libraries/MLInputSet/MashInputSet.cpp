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
/// @file MLInputSet/MashInputSet.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.08
/// @version 2.1
//------------------------------------------------------------------------------

#include "MashInputSet.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace ML;

MashInputSet::MashInputSet(Mash::IClassifierInputSet& mashInputSet,
		unsigned int maxNegatives) :
	mashInputSet_(mashInputSet), maxNegatives_(maxNegatives) {
	// There must be at least one label, heuristic, and image
	const unsigned int nbLabels = mashInputSet.nbLabels();
	nbImages_ = mashInputSet.nbImages();
	nbHeuristics_ = mashInputSet.nbHeuristics();

	assert(nbLabels > 0);
	assert(nbImages_ > 0);
	assert(nbHeuristics_ > 0);

	// Save the label, image, and coordinates of every target object in every
	// image
	unsigned int nbSamples = 0;
	unsigned int nbNegatives = 0;
	std::vector<unsigned int> labels;
	std::vector<unsigned int> images;
	std::vector < coordinates_t > coordinates;

	// For every image
	for (unsigned int i = 0; i < nbImages_; ++i) {
		// For every object in the image
		Mash::tObjectsList objects;
		mashInputSet.objectsInImage(i, &objects);

		for (Mash::tObjectsIterator obj = objects.begin(), objEnd =
				objects.end(); obj != objEnd; ++obj) {
			if (obj->target) {
				++nbSamples;
				labels.push_back(obj->label);
				images.push_back(i);
				coordinates.push_back(obj->roi_position);
			}
		}

		// Sample negatives from every image
		if (mashInputSet.isDoingDetection() && maxNegatives) {
			Mash::tCoordinatesList negatives;
			mashInputSet.negativesInImage(i, &negatives);

			// Sample at most maxNegatives positions
			const unsigned int step =
					std::max(
							static_cast<unsigned int> (negatives.size())
									/ maxNegatives, 1U);

			unsigned int nbNegs = 0;

			for (Mash::tCoordinatesIterator coord = negatives.begin() + step
					/ 2, coordEnd = negatives.end(); coord != coordEnd
					&& nbNegs < maxNegatives; coord += step) {
				++nbNegs;
				labels.push_back(nbLabels);
				images.push_back(i);
				coordinates.push_back(*coord);
			}

			nbNegatives += nbNegs;
		}
	}

	// There must be at least one sample
	assert(nbSamples > 0);

	// Sets the number of labels as max(labels) + 1 (no need for more)
	unsigned int maxPlus1 = *std::max_element(labels.begin(), labels.end()) + 1;
	assert(maxPlus1 >= nbLabels);
	nbLabelStack_.push_back(maxPlus1); // Push it twice so as to be sure to
	nbLabelStack_.push_back(maxPlus1); // never overwrite it

	// Resize the initial stack of samples
	sampleStack_.resize(1);
	sampleStack_[0].resize(nbSamples + nbNegatives);

	// Make the indices range from 0 to nbSamples - 1
	for (unsigned int s = 0; s < nbSamples + nbNegatives; ++s) {
		sampleStack_[0][s] = s;
	}

	// Sets the initial label, image, and coordinates caches
	labelCache_.swap(labels);
	imageCache_.swap(images);
	coordinatesCache_.swap(coordinates);

	// Compute the total number of features
	unsigned int nbFeatures = 0;

	// For every feature of every heuristic
	for (unsigned int h = 0; h < nbHeuristics_; ++h) {
		heuristics_.push_back(nbFeatures += mashInputSet.nbFeatures(h));
	}

	// There must be at least one feature
	assert(nbFeatures > 0);

	// Resize the initial stack of features
	featureStack_.resize(1);
	featureStack_[0].resize(nbFeatures);

	// Make the indices range from 0 to nbFeatures - 1
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		featureStack_[0][f] = f;
	}

	// Print some info about the input set to the log
	std::cout << "[MashInputSet::MashInputSet] Created input set, #samples: "
			<< nbSamples;

	if (nbNegatives) {
		std::cout << " + " << nbNegatives;
	}

	std::cout << ", #features: " << nbFeatures << ", #labels: " << nbLabels
			<< (nbNegatives ? " + 1" : "") << ", #images: " << nbImages_
			<< ", #heuristics: " << nbHeuristics_ << '.' << std::endl;
}

void MashInputSet::convert(const std::vector<unsigned int>& features,
		Mash::tFeatureList& list) const {
	// For every feature
	for (unsigned int f = 0; f < features.size(); ++f) {
		// Find the heuristic into which f falls
		unsigned int h = std::upper_bound(heuristics_.begin(),
				heuristics_.end(), features[f]) - heuristics_.begin();

		list.push_back(
				Mash::tFeature(h, features[f] - (h ? heuristics_[h - 1] : 0)));
	}
}

void MashInputSet::matrixCache(std::vector<scalar_t>& data) const {
	//	std::cout << "[MashInputSet::matrixCache] sample level: "
	//		 	  << (sampleStack_.size() - 1) << ", feature level: "
	//		 	  << (featureStack_.size() - 1) << '.'
	//			  << std::endl;

	// Get the current number of samples, features, and heuristics
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();
	const unsigned int nbHeuristics = this->nbHeuristics();

	// Resize the vector to contain the selected subset
	data.resize(nbSamples * nbFeatures);

	// Partition the feature indices by heuristic
	typedef std::pair<unsigned int, unsigned int> Pair;
	std::vector<Pair> indices(nbFeatures);

	// Convert the feature indices to the level 0
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		register unsigned int index = f;

		for (unsigned int i = featureStack_.size() - 1; i > 0; --i) {
			index = featureStack_[i][index];
		}

		indices[f] = Pair(index, f);
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
		std::vector<Pair>::iterator middle = std::partition(first,
				indices.end(),
				std::bind2nd(std::less<Pair>(), Pair(heuristics_[h], 0)));

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
	std::vector < scalar_t > temp(
			*std::max_element(selected.begin(), selected.end()));

	// Report to the log the progress in filling the cache if there is more
	// than one sample and one feature
	if (nbSamples > 1 && nbFeatures > 1) {
		std::cout << "[MashInputSet::matrixCache] Computing " << data.size()
				<< " features...";
	}

	unsigned int percentage = 100;

	// Get the images and the coordinates of the samples
	const unsigned int* images = this->images();
	const coordinates_t* coordinates = this->coordinates();

	// For all the selected samples
	for (unsigned int s = 0; s < nbSamples; ++s) {
		register unsigned int index = s;

		for (unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
			index = sampleStack_[i][index];
		}

		// Recopy the computed features at the proper place in the cache
		for (unsigned int h = 0, j = 0; h < nbHeuristics; ++h) {
			if (selected[h]) {
				// computeSomeFeatures doesn't like to compute 0 features
				assert(
						mashInputSet_.computeSomeFeatures(images[s],
								coordinates[s], h, selected[h], &indexes[j],
								&temp[0]));

				for (unsigned int k = 0; k < selected[h]; ++k) {
					assert(temp[k] == temp[k]); // Check for NaN
					data[s * nbFeatures + indices[j + k].second] = temp[k];
				}

				j += selected[h];
			}
		}

		unsigned int percent = s * 100 / nbSamples;

		if (nbSamples > 1 && nbFeatures > 1 && percent != percentage) {
			std::cout << ' ' << (percentage = percent) << '%' << std::flush;
		}
	}

	if (percentage != 100) {
		std::cout << " 100%." << std::endl;
	}
}

void MashInputSet::labelCache(std::vector<unsigned int>& data) const {
	// Get the current number of samples and images
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbImages = this->nbImages();
	const unsigned int nbLabels = mashInputSet_.nbLabels();

	// Resize the vector to contain the selected subset
	data.resize(nbSamples);

	// First save the labels of all the objects
	std::vector<unsigned int> labels;

	// For every image
	for (unsigned int i = 0; i < nbImages; ++i) {
		// For every object in the image
		Mash::tObjectsList objects;
		mashInputSet_.objectsInImage(i, &objects);

		for (Mash::tObjectsIterator obj = objects.begin(), objEnd =
				objects.end(); obj != objEnd; ++obj) {
			if (obj->target) {
				labels.push_back(obj->label);
			}
		}

		// Sample negatives from every image
		if (mashInputSet_.isDoingDetection() && maxNegatives_) {
			Mash::tCoordinatesList negatives;
			mashInputSet_.negativesInImage(i, &negatives);

			// Sample at most maxNegatives positions
			if (negatives.size() > maxNegatives_) {
				negatives.resize(maxNegatives_);
			}

			for (Mash::tCoordinatesIterator coord = negatives.begin(),
					coordEnd = negatives.end(); coord != coordEnd; ++coord) {
				labels.push_back(nbLabels);
			}
		}
	}

	// Make sure it is really all the labels
	assert(labels.size() == sampleStack_[0].size());

	// Then index into those labels (highly inefficient but ok since normally
	// the labels are cached only once in the constructor)
	for (unsigned int s = 0; s < nbSamples; ++s) {
		register unsigned int index = s;

		for (unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
			index = sampleStack_[i][index];
		}

		data[s] = labels[index];
	}
}

void MashInputSet::weightCache(std::vector<scalar_t>& data) const {
	// Resize the vector to contain the selected subset and fill it with 1's
	data.resize(nbSamples());
	std::fill(data.begin(), data.end(), 1);
}

void MashInputSet::imageCache(std::vector<unsigned int>& data) const {
	// Get the current number of samples and images
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbImages = this->nbImages();

	// Resize the vector to contain the selected subset
	data.resize(nbSamples);

	// First save the images of all the objects
	std::vector<unsigned int> images;

	// For every image
	for (unsigned int i = 0; i < nbImages; ++i) {
		// For every object in the image
		Mash::tObjectsList objects;
		mashInputSet_.objectsInImage(i, &objects);

		for (Mash::tObjectsIterator obj = objects.begin(), objEnd =
				objects.end(); obj != objEnd; ++obj) {
			if (obj->target) {
				images.push_back(i);
			}
		}

		// Sample negatives from every image
		if (mashInputSet_.isDoingDetection() && maxNegatives_) {
			Mash::tCoordinatesList negatives;
			mashInputSet_.negativesInImage(i, &negatives);

			// Sample at most maxNegatives positions
			if (negatives.size() > maxNegatives_) {
				negatives.resize(maxNegatives_);
			}

			for (Mash::tCoordinatesIterator coord = negatives.begin(),
					coordEnd = negatives.end(); coord != coordEnd; ++coord) {
				images.push_back(i);
			}
		}
	}

	// Make sure it is really all the images
	assert(images.size() == sampleStack_[0].size());

	// Then index into those images (highly inefficient but ok since normally
	// the images are cached only once in the constructor)
	for (unsigned int s = 0; s < nbSamples; ++s) {
		register unsigned int index = s;

		for (unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
			index = sampleStack_[i][index];
		}

		data[s] = images[index];
	}
}

void MashInputSet::coordinatesCache(std::vector<coordinates_t>& data) const {
	// Get the current number of samples and images
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbImages = this->nbImages();

	// Resize the vector to contain the selected subset
	data.resize(nbSamples);

	// First save the coordinates of all the objects
	std::vector < coordinates_t > coordinates;

	// For every image
	for (unsigned int i = 0; i < nbImages; ++i) {
		// For every object in the image
		Mash::tObjectsList objects;
		mashInputSet_.objectsInImage(i, &objects);

		for (Mash::tObjectsIterator obj = objects.begin(), objEnd =
				objects.end(); obj != objEnd; ++obj) {
			if (obj->target) {
				coordinates.push_back(obj->roi_position);
			}
		}

		// Sample negatives from every image
		if (mashInputSet_.isDoingDetection() && maxNegatives_) {
			Mash::tCoordinatesList negatives;
			mashInputSet_.negativesInImage(i, &negatives);

			// Sample at most maxNegatives positions
			const unsigned int step = std::max(
					(unsigned int) (negatives.size()) / maxNegatives_, 1U);
			unsigned int nbNegatives = 0;

			for (Mash::tCoordinatesIterator coord = negatives.begin(),
					coordEnd = negatives.end(); coord != coordEnd
					&& nbNegatives < maxNegatives_; coord += step) {
				++nbNegatives;
				coordinates.push_back(*coord);
			}
		}
	}

	// Make sure it is really all the coordinates
	assert(coordinates.size() == sampleStack_[0].size());

	// Then index into those coordinates (highly inefficient but ok since
	// normally the coordinates are cached only once in the constructor)
	for (unsigned int s = 0; s < nbSamples; ++s) {
		register unsigned int index = s;

		for (unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
			index = sampleStack_[i][index];
		}

		data[s] = coordinates[index];
	}
}

void MashInputSet::heuristicCache(std::vector<unsigned int>& data) const {
	// Get the current number of features
	const unsigned int nbFeatures = this->nbFeatures();

	// Resize the vector to contain the selected subset
	data.resize(nbFeatures);

	for (unsigned int f = 0; f < nbFeatures; ++f) {
		register unsigned int index = f;

		for (unsigned int i = featureStack_.size() - 1; i > 0; --i) {
			index = featureStack_[i][index];
		}

		// Find the heuristic into which f falls
		data[f] = std::upper_bound(heuristics_.begin(), heuristics_.end(),
				index) - heuristics_.begin();
	}
}

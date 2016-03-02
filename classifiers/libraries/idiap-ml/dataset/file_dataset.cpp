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


/// \file	dataset_file.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 24, 2011

#include "file_dataset.h"

#include <idiap-ml/utils/transpose.h>

#include <iostream>
#include <stdexcept>
#include <stdint.h>

using namespace ml;

FileDataSet::FileDataSet(Mash::DataReader& reader) : reader_(reader) {
	// Read the header of the file
	uint32_t header[5];
	reader.read(reinterpret_cast<uint8_t*>(header), 20);

	// Check the version of the file
	if (header[0] != 0x00010002U) {
		throw std::invalid_argument("invalid dataset file version");
	}

	// Make sure the file contains at least 2 samples, 1 feature, 2 labels, and
	// 1 heuristic
	if (header[1] < 2 || !header[2] || header[3] < 2 || !header[4]) {
		throw std::length_error("a valid dataset must constain at least 2 "
								"samples, 1 feature, 2 labels, and 1 heuristic");
	}

	// Determine the size of the file
	std::streampos size = reader.size();

	// Throw an exception if the file is too small
	uint64_t data = static_cast<uint64_t>(header[1]) * header[2] * sizeof(float);

	if (size < 20 + data + (header[1] + header[2]) * 4) {
		throw std::length_error("incomplete dataset file");
	}

	// Read the labels and the heuristics
	nbLabels_ = header[3];
	nbHeuristics_ = header[4];

	reader.seek(data);

	std::vector<uint32_t> tmp(header[1]);
	reader.read(reinterpret_cast<uint8_t*>(&tmp[0]), header[1] * 4);
	labels_.assign(tmp.begin(), tmp.end());

	tmp.resize(header[2]);
	reader_.read(reinterpret_cast<uint8_t*>(&tmp[0]), header[2] * 4);
	heuristics_.assign(tmp.begin(), tmp.end());

	// Print some info about the input set to the log
	std::cout << "[FileDataSet::FileDataSet] Created dataset, #samples: "
			  << header[1] << ", #features: " << header[2] << ", #labels: "
			  << nbLabels_ << ", #heuristics: " << nbHeuristics_ << std::endl;
}

unsigned int FileDataSet::nbSamples() const {
	return static_cast<unsigned int>(labels_.size());
}

unsigned int FileDataSet::nbFeatures() const {
	return static_cast<unsigned int>(heuristics_.size());
}

unsigned int FileDataSet::nbLabels() const {
	return nbLabels_;
}

unsigned int FileDataSet::nbHeuristics() const {
	return nbHeuristics_;
}

void FileDataSet::computeFeatures(unsigned int nbSamples,
								  const unsigned int* sampleIndices,
								  unsigned int nbFeatures,
								  const unsigned int* featureIndices,
								  float* values,
								  bool transpose) const {
	// Temporary values
	std::vector<float> tmp(labels_.size());

	// Seek to the beginning of the features
	const uint64_t stride = labels_.size() * sizeof(float);

#ifndef NDEBUG
//	for (unsigned int s = 0; s < nbSamples; ++s) {
//		if (sampleIndices[s] >= labels_.size()) {
//			throw std::out_of_range("out of range sample");
//		}
//	}
#endif

	// Report to the log the progress in filling the cache if there is more
	// than one sample and one feature
	if (nbSamples > 1 && nbFeatures > 1) {
		std::cout << "[FileDataSet::computeFeatures] Computing " << nbSamples
				  << 'x' << nbFeatures << " features...";
	}

	unsigned int percentage = 100;

	for (unsigned int f = 0; f < nbFeatures; ++f) {
#ifndef NDEBUG
//		if (featureIndices[f] >= heuristics_.size()) {
//			throw std::out_of_range("out of range feature");
//		}
#endif
		// Read all the samples
		reader_.seek(20 + featureIndices[f] * stride, Mash::DataReader::BEGIN);
		reader_.read(reinterpret_cast<uint8_t*>(&tmp[0]),
					 static_cast<std::streamsize>(stride));

		if (transpose) {
			for (unsigned int s = 0; s < nbSamples; ++s) {
				values[f * nbSamples + s] = tmp[sampleIndices[s]];
			}
		}
		else {
			for (unsigned int s = 0; s < nbSamples; ++s) {
				values[s * nbFeatures + f] = tmp[sampleIndices[s]];
			}
		}

		unsigned int percent = f * 100 / nbFeatures;

		if (nbSamples > 1 && nbFeatures > 1 && percent != percentage) {
			std::cout << ' ' << (percentage = percent) << '%' << std::flush;
		}
	}

	if (percentage != 100) {
		std::cout << " 100%" << std::endl;
	}
}

unsigned int FileDataSet::label(unsigned int sample) const {
#ifndef NDEBUG
	return labels_.at(sample);
#else
	return labels[sample];
#endif
}

unsigned int FileDataSet::heuristic(unsigned int feature) const {
#ifndef NDEBUG
	return heuristics_.at(feature);
#else
	return heuristics[feature];
#endif
}

void FileDataSet::save(const IDataSet& dataset, Mash::DataWriter& writer) {
	// Write the header
	uint32_t header[5];
	header[0] = 0x00010002U;
	header[1] = dataset.nbSamples();
	header[2] = dataset.nbFeatures();
	header[3] = dataset.nbLabels();
	header[4] = dataset.nbHeuristics();
	writer.write(reinterpret_cast<uint8_t*>(header), 20);

	// The maximum size of the cache
	const unsigned int maximumCacheSize = (1U << 30U) / sizeof(float);
	unsigned int nbFeatures = std::min(maximumCacheSize / header[1], header[2]);

	// Sample indices
	std::vector<unsigned int> sampleIndices(header[1]);

	for (unsigned int s = 0; s < header[1]; ++s) {
		sampleIndices[s] = s;
	}

	// Feature indices
	std::vector<unsigned int> featureIndices(nbFeatures);

	// Temporary values
	std::vector<float> tmp(header[1] * nbFeatures);

	// Write the features by batch
	for (unsigned int f = 0; f < header[2]; f += nbFeatures) {
		nbFeatures = std::min(nbFeatures, header[2] - f);

		for (unsigned int i = 0; i < nbFeatures; ++i) {
			featureIndices[i] = f + i;
		}

		dataset.computeFeatures(header[1], &sampleIndices[0], nbFeatures,
								&featureIndices[0], &tmp[0], true);

		writer.write(reinterpret_cast<uint8_t*>(&tmp[0]),
					 header[1] * nbFeatures * sizeof(float));
	}

	// Write the labels
	std::vector<uint32_t> labels(header[1]);

	for (unsigned int s = 0; s < header[1]; ++s) {
		labels[s] = dataset.label(s);
	}

	writer.write(reinterpret_cast<uint8_t*>(&labels[0]), header[1] * 4);

	// Write the heuristics
	labels.resize(header[2]);

	for (unsigned int f = 0; f < header[2]; ++f) {
		labels[f] = dataset.heuristic(f);
	}

	writer.write(reinterpret_cast<uint8_t*>(&labels[0]), header[2] * 4);
}

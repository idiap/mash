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


/// \file	mash_dataset.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 21, 2011

#ifndef ML_MASH_DATASET_H
#define ML_MASH_DATASET_H

#include "dataset_interface.h"

#include <mash-classification/classifier.h>

#include <vector>

namespace ml {
	/// \brief	Mash::IClassifierInputSet wrapper class
	class MashDataSet : public IDataSet {
	public:
		/// \brief	Constructor
		/// \param	mashInputSet	The IClassifierInputSet to wrap
		/// \param	maxNegatives	Maximum number of negative samples to
		///							extract from an image (default 10)
		MashDataSet(Mash::IClassifierInputSet* mashIClassifierInputSet,
					unsigned int maxNegatives = 10);

		/// \brief	Returns the total number of samples
		virtual unsigned int nbSamples() const;

		/// \brief	Returns the total number of features
		virtual unsigned int nbFeatures() const;

		/// \brief	Returns (an upper-bound on) the number of distinct labels
		virtual unsigned int nbLabels() const;

		/// \brief	Returns (an upper-bound on) the number of distinct
		///			heuristics (feature extractors)
		virtual unsigned int nbHeuristics() const;

		/// \brief	Computes the values of a selected subset of features
		///			associated with a selected subset of samples
		/// \param	nbSamples		Number of samples to compute
		/// \param	sampleIndices	Indices of the samples to compute
		/// \param	nbFeatures		Number of features to compute
		/// \param	featureIndices	Indices of the features to compute
		/// \retval values			The computed features
		/// \param	transpose		Whether or not to transpose the values
		virtual void computeFeatures(unsigned int nbSamples,
									 const unsigned int* sampleIndices,
									 unsigned int nbFeatures,
									 const unsigned int* featureIndices,
									 float* values,
									 bool transpose = false) const;

		/// \brief	Returns the label of a \param sample
		/// \remark Labels always lie in the range [0, %nbLabels)
		virtual unsigned int label(unsigned int sample) const;

		/// \brief	Returns the heuristic of a \param feature
		/// \remark Heuristics always lie in the range [0, %nbHeuristics)
		virtual unsigned int heuristic(unsigned int feature) const;

		/// \brief	Add a new sample at the end of the dataset
		/// \param	label		Label of the sample
		/// \param	image		Index of the image
		/// \param	position	Position in the image
		virtual void pushSample(unsigned int label,
								unsigned int image,
								Mash::coordinates_t position);

		/// \brief	Remove the sample at the end of the dataset
		virtual void popSample();

		/// \brief	Converts between internal feature indices and
		///			Mash::tFeature's
		/// \param	features	The vector of internal feature indices
		/// \retval list		The output list of Mash::tFeature's
		virtual void convert(const std::vector<unsigned int>& features,
					 		 Mash::tFeatureList& list) const;

		/// \brief	Converts between Mash::tFeature's and internal feature
		///			indices
		/// \param	list		The input list of Mash::tFeature's
		/// \retval	features	The output vector of internal feature indices
		virtual void convert(const Mash::tFeatureList& list,
					 		 std::vector<unsigned int>& features) const;

	protected:
		/// A pointer to the underlying Mash input set
		Mash::IClassifierInputSet* inputset_;

		/// The maximum number of negatives to extract from an image
		unsigned int maxNegatives_;

		unsigned int nbLabels_; ///< The number of distinct labels
		std::vector<unsigned int> labels_; ///< Sample labels
		std::vector<unsigned int> images_; ///< Sample images
		std::vector<Mash::coordinates_t> coordinates_; ///< Sample coordinates

		/// The range of features of every heuristics (useful to convert
		/// internal feature indices to Mash tFeature's)
		std::vector<unsigned int> heuristics_;
	};
} // namespace ml

#endif // ML_MASH_DATASET_H

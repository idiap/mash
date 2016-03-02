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


/// \file	dataset_interface.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 19, 2011

#ifndef ML_DATASET_INTERFACE_H
#define ML_DATASET_INTERFACE_H

namespace ml {
	/// \brief	The base interface that all data-sets must re-define
	class IDataSet {
	public:
		/// \brief	Destructor
		virtual ~IDataSet() {};

		/// \brief	Returns the total number of samples
		virtual unsigned int nbSamples() const = 0;

		/// \brief	Returns the total number of features
		virtual unsigned int nbFeatures() const = 0;

		/// \brief	Returns (an upper-bound on) the number of distinct labels
		virtual unsigned int nbLabels() const = 0;

		/// \brief	Returns (an upper-bound on) the number of distinct
		///			heuristics (feature extractors)
		virtual unsigned int nbHeuristics() const = 0;

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
									 bool transpose = false) const = 0;

		/// \brief	Returns the label of a \param sample
		/// \remark Labels always lie in the range [0, %nbLabels)
		virtual unsigned int label(unsigned int sample) const = 0;

		/// \brief	Returns the heuristic of a \param feature
		/// \remark Heuristics always lie in the range [0, %nbHeuristics)
		virtual unsigned int heuristic(unsigned int feature) const = 0;
	};
} // namespace ml

#endif // ML_DATASET_INTERFACE_H

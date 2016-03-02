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
/// @file MLInputSet/MashInputSet.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.08
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_MASHINPUTSET_H
#define ML_INPUTSET_MASHINPUTSET_H

#include "InputSet.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	IClassifierInputSet wrapper class
	///
	/// Simplify the implementation of classifiers:
	/// - Classifiers have to deal only with samples/features (no image or
	///   heuristic stuff, although they can still know the label, image, and
	///   coordinates of every sample and the heuristic of every feature)
	/// - They can transparently select a subset of samples and features,
	///   modify the labels, assign weights to the samples, etc.
	/// - The input set supports different kind of sample/feature/heuristic
	///   selection and normalization through external filters
	/// - It handles object detection easily (simply add an extra label for the
	///   negatives)
	//--------------------------------------------------------------------------
	class MashInputSet : public InputSet {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	mashInputSet	The IClassifierInputSet to wrap
		/// @param  maxNegatives	Number of negatives to extract from an image
		///							(default 10)
		//----------------------------------------------------------------------
		MashInputSet(Mash::IClassifierInputSet& mashInputSet,
					 unsigned int maxNegatives = 10);

		//_____ Public methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Converts between internal feature indices to
		///			Mash::tFeature's
		///
		/// @param	features	The vector of internal feature indices
		/// @retval	list		The output list of Mash::tFeature's
		//----------------------------------------------------------------------
		void convert(const std::vector<unsigned int>& features,
					 Mash::tFeatureList& list) const;

		//_____ Methods to implement _______
	protected:
		//----------------------------------------------------------------------
		/// @brief	Creates the first matrix cache
		//----------------------------------------------------------------------
		virtual void matrixCache(std::vector<scalar_t>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first label cache
		//----------------------------------------------------------------------
		virtual void labelCache(std::vector<unsigned int>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first weight cache
		//----------------------------------------------------------------------
		virtual void weightCache(std::vector<scalar_t>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first image cache
		//----------------------------------------------------------------------
		virtual void imageCache(std::vector<unsigned int>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first coordinates cache
		//----------------------------------------------------------------------
		virtual void coordinatesCache(std::vector<coordinates_t>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first heuristic cache
		//----------------------------------------------------------------------
		virtual void heuristicCache(std::vector<unsigned int>& data) const;

		//_____ Attributes _______
	protected:
		/// A reference to the underlying Mash input set
		Mash::IClassifierInputSet& mashInputSet_;

		/// The maximum number of negatives to extract from an image
		unsigned int maxNegatives_;

		/// The range of features of every heuristics (useful to convert
		/// internal feature indices to Mash tFeature's)
		std::vector<unsigned int> heuristics_;
	};
} // namespace ML

#endif // ML_INPUTSET_MASHINPUTSET_H

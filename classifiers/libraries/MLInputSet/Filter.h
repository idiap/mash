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
/// @file MLInputSet/Filter.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.05.31
/// @version 1.5
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_FILTER_H
#define ML_INPUTSET_FILTER_H

#include "InputSet.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Filter base class
	///
	/// Apply a filter (can be feature or sample selection, normalization or any
	/// transformation, for example PCA) on an input set.
	//--------------------------------------------------------------------------
	class Filter : public InputSet {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	inputSet	The input set to filter
		///
		/// Initializes the filter (create sample and feature stacks from the
		/// top input set stacks)
		//----------------------------------------------------------------------
		Filter(InputSet& inputSet);

		//_____ Public methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief Converts between filter features to the underlying input set
		///		   features
		//----------------------------------------------------------------------
		virtual void convert(std::vector<unsigned int>& features) const;

		//----------------------------------------------------------------------
		/// @brief	Clear all the caches completely
		//----------------------------------------------------------------------
		virtual void clear();

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
		InputSet& inputSet_; ///< A reference to the underlying input set
	};
} // namespace ML

#endif // ML_INPUTSET_FILTER_H

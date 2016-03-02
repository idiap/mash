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
/// @file MLInputSet/Statistical.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.06.17
/// @version 1.5
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_STATISTICAL_H
#define ML_INPUTSET_STATISTICAL_H

#include "Filter.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Statistical normalization filter
	///
	/// Normalizes all the features to 0 mean, unit standard deviation.
	//--------------------------------------------------------------------------
	class Statistical : public Filter {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor, takes a reference to the underlying input set
		//----------------------------------------------------------------------
		Statistical(InputSet& inputSet);

		//_____ Methods to implement _______
	private:
		//----------------------------------------------------------------------
		/// @brief	Creates the first feature cache
		//----------------------------------------------------------------------
		virtual void matrixCache(std::vector<scalar_t>& data) const;

		//_____ Attributes _______
	private:
		/// Used to normalize the input set features
		std::vector<scalar_t> means_;
		std::vector<scalar_t> stds_;
	};
} // namespace ML

#endif // ML_INPUTSET_STATISTICAL_H

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
/// @file MLInputSet/Energy.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.07.01
/// @version 1.5
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_ENERGY_H
#define ML_INPUTSET_ENERGY_H

#include "Filter.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Energy normalization filter
	///
	/// Normalizes all the sample to unit L^k norm.
	//--------------------------------------------------------------------------
	class Energy : public Filter {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor, takes a reference to the underlying input set
		///			and the desired L^k norm (default 2)
		//----------------------------------------------------------------------
		Energy(InputSet& inputSet,
			   unsigned int k = 2);

		//_____ Methods to implement _______
	private:
		//----------------------------------------------------------------------
		/// @brief	Creates the first feature cache
		//----------------------------------------------------------------------
		virtual void matrixCache(std::vector<scalar_t>& data) const;

		//_____ Internal methods _______
	private:
		//----------------------------------------------------------------------
		/// @brief	Computes the norms of the samples
		//----------------------------------------------------------------------
		void computeNorms();

		//_____ Attributes _______
	private:
		unsigned int k_; ///< The L^k norm to use
		std::vector<scalar_t> norms_; ///< The norms of the samples
	};
} // namespace ML

#endif // ML_INPUTSET_ENERGY_H

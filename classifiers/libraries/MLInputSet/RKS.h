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
/// @file MLInputSet/RKS.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.06.01
/// @version 1.5
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_RKS_H
#define ML_INPUTSET_RKS_H

#include "Filter.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Random Kitchen Sinks (transforms a non-linear problem to a
	///			linear one by projecting the data with a random matrix)
	///
	/// Reference: Ali Rahimi, Benjamin Recht: Weighted Sums of Random Kitchen
	/// Sinks: Replacing minimization with randomization in learning. NIPS 2008.
	//--------------------------------------------------------------------------
	class RKS : public Filter {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	gamma	Similar as the gamma parameter of an SVM with an RBF
		///					(Gaussian) kernel (there is no cross-validation
		///					option to automatically set it as it wouldn't be
		///					much more efficient than training it several times
		///					with different gamma's)
		/// @param	nbProjections	The number of random projections (similar to
		///							weak classifiers in Boosting) to generate
		///							(default 1000)
		//----------------------------------------------------------------------
		RKS(InputSet& inputSet,
			scalar_t gamma,
			unsigned int nbProjections = 1000);

		//_____ Methods to implement _______
	private:
		//----------------------------------------------------------------------
		/// @brief	Creates the first feature cache
		//----------------------------------------------------------------------
		virtual void matrixCache(std::vector<scalar_t>& data) const;

		//----------------------------------------------------------------------
		/// @brief	Creates the first heuristic cache
		//----------------------------------------------------------------------
		virtual void heuristicCache(std::vector<unsigned int>& data) const;

		//_____ Private methods _______
	private:
		scalar_t randn() const;

		//_____ Attributes _______
	private:
		unsigned int nbProjections_;
		std::vector<scalar_t> projections_;
		std::vector<scalar_t> bias_;

		// Used by the randn() method
		mutable scalar_t tmp_;
		mutable bool cached_;
	};
} // namespace ML

#endif // ML_INPUTSET_RKS_H

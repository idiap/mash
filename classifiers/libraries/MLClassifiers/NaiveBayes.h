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
/// @file MLClassifiers/NaiveBayes.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.01.26
/// @version 1.7
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_NAIVEBAYES_H
#define ML_CLASSIFIERS_NAIVEBAYES_H

#include "Classifier.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Naive Bayes' classifier
	///
	/// Assumes that the features are independent and can be well approximated
	/// by Gaussians.
	//--------------------------------------------------------------------------
	class NaiveBayes : public Classifier {
		//_____ Methods to implement _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Clone method
		///
		/// @return	A deep copy of the classifier
		//----------------------------------------------------------------------
		virtual Classifier* clone() const;

		//----------------------------------------------------------------------
		/// @brief	Trains the classifier
		///
		/// @param	inputSet	The input set over which the classifier should
		/// 					be trained
		//----------------------------------------------------------------------
		virtual void train(InputSet& inputSet);

		//----------------------------------------------------------------------
		/// @brief	Predicts the class memberships for a given sample
		///
		/// @param	inputSet	The input set containing the sample to classify
		/// @param	sample 		The index of the sample to classify
		/// @retval	distr		The predicted class memberships
		//----------------------------------------------------------------------
		virtual void distribution(InputSet& inputSet,
								  unsigned int sample,
								  scalar_t* distr) const;

		//----------------------------------------------------------------------
		/// @brief	Outputs the classifier to a stream
		///
		/// @param	out	The output stream
		//----------------------------------------------------------------------
		virtual void print(std::ostream& out) const;

		//----------------------------------------------------------------------
		/// @brief	Reports the features used by the classifier
		///
		/// @retval	features	The vector of features to populate
		//----------------------------------------------------------------------
		virtual void report(std::vector<unsigned int>& features) const;

		//_____ Attributes _______
	private:
		/// The means of every feature for every label
		std::vector<scalar_t> means_;

		/// The standard deviations of every feature for every label
		std::vector<scalar_t> stds_;

		/// The probability priors of every label
		std::vector<scalar_t> priors_;
	};
} // namespace ML

#endif // ML_CLASSIFIERS_NAIVEBAYES_H

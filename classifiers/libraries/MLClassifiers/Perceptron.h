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
/// @file MLClassifiers/Perceptron.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.05
/// @version 1.5
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_PERCEPTRON_H
#define ML_CLASSIFIERS_PERCEPTRON_H

#include "Classifier.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Perceptron classifier
	///
	/// Separates the classes using hyperplanes. Implements the standard
	/// perceptron algorithm (updates all the weights at the end of a step) as
	/// well as the simpler stochastic variant updating the weights immediately
	/// in case of missclassification (reward and punishment scheme). It uses
	/// the Pocket trick (caches the best weight vector computed so far in terms
	/// of training error to ensure stability) and Kesler's construction to deal
	/// with the multi-class case.
	//--------------------------------------------------------------------------
	class Perceptron : public Classifier {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	stochastic	Chooses between the standard perceptron
		///						algorithm and the simpler stochastic variant
		///						(default stochastic)
		/// @param	decay		The factor by which the learning rate is
		///						decreased at each step (default 0.99)
		/// @param	nbSteps		The number of steps without update of the pocket
		///						after which convergence is assumed (default 10)
		//----------------------------------------------------------------------
		Perceptron(bool stochastic = true,
				   scalar_t decay = 0.99,
				   unsigned int nbSteps = 10);

		//_____ Methods inherited from Classifier _______
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

		//_____ Attributes _______
	private:
		/// Choose between the standard perceptron algorithm if false or the
		/// simpler stochastic variant otherwise
		bool stochastic_;

		/// The factor by which the learning rate is decreased at each step
		scalar_t decay_;

		/// The number of steps without update of the pocket after which
		/// convergence is assumed
		unsigned int nbSteps_;

		/// The normal vectors of the hyperplanes
		std::vector<scalar_t> w_;
	};
} // namespace ML

#endif // ML_CLASSIFIERS_PERCEPTRON_H

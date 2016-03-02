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
/// @file MLClassifiers/LinearSVM.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.07.09
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_LINEARSVM_H
#define ML_CLASSIFIERS_LINEARSVM_H

#include "Classifier.h"

// Internally uses liblinear (dense and weighted, version 1.7)
#include <liblinear/linear.h>

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Linear Support Vector Machine classifier
	///
	/// Separates the two classes using a hyperplanes. Internally uses liblinear
	/// (dense and weighted version, edited to use scalar_t and Eigen).
	//--------------------------------------------------------------------------
	class LinearSVM : public Classifier {
		//_____ Internal types _______
	public:
		/// Type of the solver to use.
		enum Solver {L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC,
					 L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR,
					 L2R_LR_DUAL};

		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	solver	The type of the solver to use:
		///					- L2-regularized logistic regression
		///					- L2-regularized L2-loss support vector
		///					  classification (dual)
		///					- L2-regularized L2-loss support vector
		///					  classification (primal)
		///					- L2-regularized L1-loss support vector
		///					  classification (dual)
		///					- Multi-class support vector classification by
		///					  Crammer and Singer
		///					- L1-regularized L2-loss support vector
		///					  classification
		///					- L1-regularized logistic regression
		///					- L2-regularized logistic regression (dual)
		///					(default L2R_LR)
		/// @param	cnu		Sets the regularization parameter C (if negative it
		///					is automatically selected using 5 folds
		///					cross-validation, default -1)
		//----------------------------------------------------------------------
		LinearSVM(Solver solver = L2R_LR,
				  scalar_t cnu = -1);

		//----------------------------------------------------------------------
		/// @brief	Destructor
		///
		/// Necessary to delete properly liblinear's variables.
		//----------------------------------------------------------------------
		virtual ~LinearSVM();

		//----------------------------------------------------------------------
		/// @brief	Copy constructor
		///
		/// Necessary to copy properly liblinear's variables.
		//----------------------------------------------------------------------
		LinearSVM(const LinearSVM& svm);

		//----------------------------------------------------------------------
		/// @brief	Assignment operator
		///
		/// Necessary to assign properly libsvm's variables.
		//----------------------------------------------------------------------
		LinearSVM& operator=(const LinearSVM& svm);

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
		// liblinear variables
		parameter parameters_;
		model* model_;

		/// Maps liblinear's labels to the normal labels
		std::vector<int> map_;

		/// Indices of the selected features (in case there is too much)
		std::vector<unsigned int> indices_;
	};
} // namespace ML

#endif // ML_CLASSIFIERS_LINEARSVM_H

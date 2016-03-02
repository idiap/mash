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
/// @file MLClassifiers/SVM.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.29
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_SVM_H
#define ML_CLASSIFIERS_SVM_H

#include "Classifier.h"

// Internally uses libsvm (dense and weighted, version 3.0)
#include <libsvm/svm.h>

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Support Vector Machine classifier
	///
	/// Separates the two classes using hyperplanes in a high-dimensional space.
	/// Can use: - linear: u' * v
	///			 - polynomial: (gamma * u' * v + coef0)^degree
	///			 - radial basis function: exp(-gamma * |u - v|^2)
	///			 - sigmoid: tanh(gamma * u' * v + coef0)
	/// kernels. Internally uses libsvm (dense and weighted version, edited to
	/// use scalar_t and Eigen).
	//--------------------------------------------------------------------------
	class SVM : public Classifier {
		//_____ Internal types _______
	public:
		/// Type of the kernel to use.
		enum Kernel {LINEAR, POLY, RBF, SIGMOID};

		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	nusvm	Sets the SVM to be a nu-SVM rather than a C-SVM
		///					(default C-SVM)
		/// @param	kernel	The type of the kernel to use (linear, polynomial,
		///					RBF or sigmoid, default RBF)
		/// @param	degree	Sets degree in the kernel function (default 3)
		/// @param	gamma	Sets gamma in the kernel function (if negative it is
		///					automatically selected using 5 folds
		///					cross-validation, default -1)
		/// @param	coef0	Sets coef0 in the kernel function (default 1)
		/// @param	cnu		Sets the regularization parameter C if the svm is a
		///					C-SVM, nu if it is a nu-SVM (if negative it is
		///					automatically selected using 5 folds
		///					cross-validation, default -1)
		//----------------------------------------------------------------------
		SVM(bool nusvm = false,
			Kernel kernel = RBF,
			unsigned int degree = 3,
			scalar_t gamma = -1,
			scalar_t coef0 = 1,
			scalar_t cnu = -1);

		//----------------------------------------------------------------------
		/// @brief	Destructor
		///
		/// Necessary to delete properly libsvm's variables.
		//----------------------------------------------------------------------
		virtual ~SVM();

		//----------------------------------------------------------------------
		/// @brief	Copy constructor
		///
		/// Necessary to copy properly libsvm's variables.
		//----------------------------------------------------------------------
		SVM(const SVM& svm);

		//----------------------------------------------------------------------
		/// @brief	Assignment operator
		///
		/// Necessary to assign properly libsvm's variables.
		//----------------------------------------------------------------------
		SVM& operator=(const SVM& svm);

		//_____ Methods inherited from Classifier _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Clone method
		///
		/// @return A deep copy of the classifier
		//----------------------------------------------------------------------
		Classifier* clone() const;

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
		/// libsvm variables
		svm_problem problem_;
		svm_parameter parameters_;
		svm_model* model_;

		/// Vectors used to store the problem's features and labels
		std::vector<scalar_t> matrix_;
		std::vector<svm_node> samples_;
		std::vector<double> labels_;

		/// Maps libsvm's labels to the normal labels
		std::vector<int> map_;
	};
} // namespace ML

#endif // ML_CLASSIFIERS_SVM_H

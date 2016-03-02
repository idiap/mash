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
/// @file MLClassifiers/Classifier.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.04
/// @version 1.7
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_CLASSIFIER_H
#define ML_CLASSIFIERS_CLASSIFIER_H

#include <MLInputSet/InputSet.h>

#include <iosfwd>

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Classifier abstract base class
	///
	/// Redefines a new interface in addition to the one of
	/// mash-classification/classifier.h to simplify the implementation of
	/// classifiers and extend their functionalities (mostly through the use of
	/// the %ML::InputSet and the %ML::Filter classes).
	//--------------------------------------------------------------------------
	class Classifier {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Destructor
		///
		/// Virtual destructor so as to make sure that the destructors of
		/// derived classes will also get called.
		//----------------------------------------------------------------------
		virtual ~Classifier();

		//_____ Methods to implement _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Clone method
		///
		/// @return	A deep copy of the classifier
		///
		/// @remark	All derived class must implement the method (as this is
		/// 		required by meta-classifiers such as %AdaBoost or %OneVsAll)
		//----------------------------------------------------------------------
		virtual Classifier* clone() const = 0;

		//----------------------------------------------------------------------
		/// @brief	Trains the classifier
		///
		/// @param	inputSet	The input set over which the classifier should
		/// 					be trained
		///
		/// @remark	All derived class must implement the method
		/// @remark	%inputSet is not const as the classifier may want to call
		///			one of the %push methods, and passage by reference is faster
		///	@remark Classifiers are REQUIRED to call the appropriate number of
		///			times %pop to put the input set back to its initial state
		//----------------------------------------------------------------------
		virtual void train(InputSet& inputSet) = 0;

		//----------------------------------------------------------------------
		/// @brief	Classifies the given sample
		///
		/// @param	inputSet	The input set containing the sample to classify
		/// @param	sample 		The index of the sample to classify
		/// @return				The label of the predicted class
		///
		/// @remark	A classifier MUST implement either this method or
		///			%distribution
		/// @remark	%inputSet is not const as the classifier may want to call
		///			one of the %push methods, and passage by reference is faster
		/// @remark	Classifiers are REQUIRED to call the appropriate number of
		///			times %pop to put the input set back to its initial state
		//----------------------------------------------------------------------
		virtual unsigned int classify(InputSet& inputSet,
		                              unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Predicts the class memberships for a given sample
		///
		/// @param	inputSet	The input set containing the sample to classify
		/// @param	sample 		The index of the sample to classify
		/// @retval	distr		The predicted class memberships
		///
		/// @remark	A classifier MUST implement either this method or %classify
		/// @remark	%inputSet is not const as the classifier may want to call
		///			one of the %push methods, and passage by reference is faster
		/// @remark	Classifiers are REQUIRED to call the appropriate number of
		///			times %pop to put the input set back to its initial state
		/// @remark	All the coefficients of the distribution must be positive
		//----------------------------------------------------------------------
		virtual void distribution(InputSet& inputSet,
								  unsigned int sample,
								  scalar_t* distr) const;

		//----------------------------------------------------------------------
		/// @brief	Outputs the classifier to a stream
		///
		/// @param	out	The output stream
		///
		/// @remark	It is optional but recommended for a derived class to
		///			implement the method (as it prints 'Unknown classifier.' by
		///			default)
		//----------------------------------------------------------------------
		virtual void print(std::ostream& out) const;

		//----------------------------------------------------------------------
		/// @brief	Reports the features used by the classifier
		///
		/// @retval	features	The vector of features to populate
		///
		/// @remark	It is optional but recommended for a derived class to
		///			implement the method (as it reports nothing by default)
		//----------------------------------------------------------------------
		virtual void report(std::vector<unsigned int>& features) const;
	};
} // namespace ML

#endif // ML_CLASSIFIERS_CLASSIFIER_H

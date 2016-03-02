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
/// @file MLClassifiers/NearestNeighbor.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.03.12
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_NEARESTNEIGHBOR_H
#define ML_CLASSIFIERS_NEARESTNEIGHBOR_H

#include "Classifier.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Nearest neighbor classifier
	///
	/// Predicts the label of a sample according to the labels of its nearest
	/// neighbors (according to the L^k norm).
	//--------------------------------------------------------------------------
	class NearestNeighbor : public Classifier {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	nbNeighbors	The number of neighbors to consider when
		///						determining the label of a sample (default 1)
		/// @param	Lk			The L^k norm to use to compute distances
		///						(default 2)
		//----------------------------------------------------------------------
		NearestNeighbor(unsigned int nbNeighbors = 1,
						unsigned int Lk = 2);

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
		/// @brief	Classifies the given sample
		///
		/// @param	inputSet	The input set containing the sample to classify
		/// @param	sample 		The index of the sample to classify
		/// @return				The label of the predicted class
		//----------------------------------------------------------------------
		virtual unsigned int classify(InputSet& inputSet,
									  unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Outputs the classifier to a stream
		///
		/// @param	out	The output stream
		//----------------------------------------------------------------------
		virtual void print(std::ostream& out) const;

		//_____ Attributes _______
	private:
		/// The number of neighbors to consider when determining the label of a
		/// sample
		unsigned int nbNeighbors_;

		/// The L^k norm to use to compute distances
		unsigned int Lk_;

		/// The matrix of features of the training samples
		std::vector<scalar_t> matrix_;

		/// The labels of the training samples
		std::vector<unsigned int> labels_;
	};
} // namespace ml

#endif // ML_CLASSIFIER_NEARESTNEIGHBOR_H

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


/// \file	adaboost_weaktree.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 28, 2011

#ifndef ML_ADABOOST_WEAKTREE_H
#define ML_ADABOOST_WEAKTREE_H

#include <idiap-ml/classifier_interface.h>

namespace ml {
	/// \brief	The only weak classifier that can be used by AdaBoost
	class WeakTree {
	public:
		/// \brief	Constructor
		/// \param	maxDepth	Maximum depth of the trees (default
		///						ceil(log2(#labels))
		WeakTree(unsigned int maxDepth = 0);

		/// \brief	Trains the classifier
		/// \param	dataset	The data-set over which the classifier shoud be
		///					trained
		/// \param	weights	Weights matrix (weight of the labels per sample)
		void train(const IDataSet& dataset,
				   const std::vector<std::vector<double> >& weights);

		/// \brief	Classifies the given sample
		/// \param	dataset The data-set containing the sample to classify
		/// \param	sample	The index of the sample to classify
		/// \return			The label of the predicted class
		unsigned int classify(const IDataSet& dataset,
							  unsigned int sample) const;

		/// \brief	Reports the features used by the classifier
		/// \param[out]	features	The vector of features to populate
		void report(std::vector<unsigned int>& features) const;

		/// \brief	Loads a previously saved classifier from a json::Value
		/// \param[in]	value	The input json::Value
		void load(const json::Value& value);

		/// \brief	Saves the classifier into a json::Value
		/// \param[out]	value	The output json::Value
		void save(json::Value& value) const;

		/// \brief	Outputs the classifier to a stream in a human readable form
		/// \param[out]	out	The output stream
		void print(std::ostream& out) const;

		/// \brief	Remaps the features used internally by the weak learner
		/// \param	mapping The map of features to use for the mapping
		void remap(const std::vector<unsigned int>& mapping);

	private:
		// Used to train the tree recursively
#if 0
		bool train(const IDataSet& dataset,
				   const double** weights,
				   unsigned int depth);
#else
		bool train(const float** samples,
				   const double** weights,
				   unsigned int* indices,
				   unsigned int nbSamples,
				   unsigned int nbFeatures,
				   unsigned int nbLabels,
				   unsigned int depth);
#endif
		// Used to print the tree recursively
		void print(std::ostream& out,
				   int index,
				   unsigned int depth) const;

		struct Node {
			unsigned int feature_; ///< The feature used for classification
			float split_; ///< The split point along the feature
			int labels_[2]; ///< Predicted labels
		};

		unsigned int maxDepth_;
		std::vector<Node> nodes_;
	};
} // namespace ml

#endif // ML_ADABOOST_WEAKTREE_H

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
/// \date	Apr 4, 2011

#ifndef ML_ADABOOST_MH_WEAKTREE_H
#define ML_ADABOOST_MH_WEAKTREE_H

#include <idiap-ml/classifier_interface.h>

namespace ml {
	/// \brief	The only weak classifier that can be used by AdaBoostMH
	class WeakTreeMH {
	public:
		/// \brief	Constructor
		/// \param	maxDepth	Maximum depth of the trees (default
		///						ceil(log2(#labels))
		WeakTreeMH(unsigned int maxDepth = 0);

		/// \brief	Trains the classifier
		/// \param	dataset	The data-set over which the classifier shoud be
		///					trained
		/// \param	costs	Cost matrix (cost of the labels per sample)
		void train(const IDataSet& dataset,
				   const std::vector<std::vector<double> >& costs);

		/// \brief	Predicts the class memberships for a given sample
		/// \param	dataset The data-set containing the sample to classify
		/// \param	sample	The index of the sample to classify
		/// \retval distr	The predicted class memberships
		void distribution(const IDataSet& dataset,
						  unsigned int sample,
						  char* distr) const;

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
		void train(const IDataSet& dataset,
				   const double** costs,
				   unsigned int depth);
#else
		void train(const float** samples,
				   const unsigned int* labels,
				   const double** costs,
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
			std::vector<bool> signs_; ///< The signs for every label
			unsigned int children_[2]; ///< The children of the node
		};

		unsigned int maxDepth_;
		std::vector<Node> nodes_;
	};
} // namespace ml

#endif // ML_ADABOOST_MH_WEAKTREE_H

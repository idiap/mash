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
/// @file MLClassifiers/C45Tree.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.01.21
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_C45TREE_H
#define ML_CLASSIFIERS_C45TREE_H

#include "Classifier.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	C4.5 decision tree classifier
	///
	/// Does classification based on an entropy heuristic and pruning using
	/// confidence limits based on the number of misclassifications relatively
	/// to the total number of samples in a branch.
	//--------------------------------------------------------------------------
	class C45Tree : public Classifier {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	minWeight	Minimum weight of a leaf (default 2)
		/// @param	confidence	The confidence level used when pruning the tree
		/// 					(no pruning occur if 1, default 0.25)
		/// @param	maxDepth	The maximum depth of the tree (1 yields a
		///						decision stump, default log(#labels))
		//----------------------------------------------------------------------
		C45Tree(scalar_t minWeight = 2,
				scalar_t confidence = 0.25,
				unsigned int maxDepth = 0);

		//----------------------------------------------------------------------
		/// @brief	Destructor
		//----------------------------------------------------------------------
		virtual ~C45Tree();

		//----------------------------------------------------------------------
		/// @brief	Copy constructor
		//----------------------------------------------------------------------
		C45Tree(const C45Tree& c45tree);

		//----------------------------------------------------------------------
		/// @brief	Assignment operator
		//----------------------------------------------------------------------
		C45Tree& operator=(const C45Tree& c45tree);

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
		///						be trained
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

		//----------------------------------------------------------------------
		/// @brief	Remaps the features used internally by the tree
		///
		/// @param	mapping	The map of features to use for the mapping
		//----------------------------------------------------------------------
		virtual void remap(const std::vector<unsigned int>& mapping);

		//_____ Internal methods _______
	protected:
		//----------------------------------------------------------------------
		/// @brief	Used by the train method to create subtrees recursively
		///
		/// @param	inputSet	Same as in the train method
		/// @param	indices		Indices of the samples to consider
		/// @param	nbSamples	Number of samples
		//----------------------------------------------------------------------
		void make(const InputSet& inputSet,
				  unsigned int* indices,
				  unsigned int nbSamples);

		//----------------------------------------------------------------------
		/// @brief	Calculate the uncertainty of a subtree
		///
		/// @param	sumWeights	The sum of the weights of the subtree
		/// @param	error		The error if the subtree was a leaf
		/// @return				The uncertainty
		//----------------------------------------------------------------------
		scalar_t addErrs(scalar_t sumWeights,
						 scalar_t error) const;

		//----------------------------------------------------------------------
		/// @brief	Used by the train method to prune the subtrees recursively
		///
		/// @param	inputSet	Same as in the train method
		/// @param	indices		Indices of the samples to consider
		/// @param	nbSamples	Number of samples
		/// @param	update		Whether to update the subtree or not
		/// @return				The estimated error of the subtree
		//----------------------------------------------------------------------
		scalar_t prune(const InputSet& inputSet,
					   unsigned int* indices,
					   unsigned int nbSamples,
					   bool update);

		//----------------------------------------------------------------------
		/// @brief	Prints the subtrees recursively
		///
		/// @param	out		The output stream
		/// @param	depth	The depth of the current subtree
		//----------------------------------------------------------------------
		void printRec(std::ostream& out,
					  unsigned int depth) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the size (number of non leaf nodes) of the tree
		//----------------------------------------------------------------------
		unsigned int size() const;

		//_____ Attributes _______
	protected:
		scalar_t minWeight_;	///< Minimum weight of a leaf
		scalar_t confidence_;	///< The confidence level used during pruning
		unsigned int maxDepth_;	///< The maximum depth of the tree

		unsigned int feature_;	///< The feature used for classification
		scalar_t split_;		///< The split point along the feature
		unsigned int label_;	///< Predicted label (mode of distribution)
		scalar_t sumWeights_;	///< Sum of the weights (sum of distribution)

		/// The class label distribution (only if the tree is a leaf)
		std::vector<scalar_t> distr_;

		/// The two children of the tree (if null the tree is a leaf)
		C45Tree* children_[2];
	};
} // namespace ML

#endif // ML_CLASSIFIERS_C45TREE_H

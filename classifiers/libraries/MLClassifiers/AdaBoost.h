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
/// @file MLClassifiers/AdaBoost.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.09
/// @version 2.2
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_ADABOOST_H
#define ML_CLASSIFIERS_ADABOOST_H

#include "Classifier.h"

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Discrete multiclass AdaBoost algorithm
	//--------------------------------------------------------------------------
	class AdaBoost : public Classifier {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @param	nbRounds				The number of desired boosting
		///									rounds (default 100)
		/// @param	nbFeaturesPerHeuristic	The number of features to sample at
		///									each round from each heuristic
		///									(0 means all, default 10)
		/// @param	maxDepth				Maximum depth of the trees (default
		///									ceil(log2(#labels))
		/// @param	tca						The threshold on alpha of the
		///									totally corrective step
		///									(default 0, 0 to cancel TCA)
		//----------------------------------------------------------------------
		AdaBoost(unsigned int nbRounds = 100,
				 unsigned int nbFeaturesPerHeuristic = 10,
				 unsigned int maxDepth = 0,
				 double tca = 0);

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

		//_____ Internal types _______
	private:
		//----------------------------------------------------------------------
		/// @brief	The only weak classifier that can be used
		///
		/// The interface looks furiously like the one of normal classifiers,
		///	why change something that works?
		//----------------------------------------------------------------------
		class WeakTree {
			//_____ Construction / Destruction and Copy _______
		public:
			//------------------------------------------------------------------
			/// @brief	Constructor
			///
			/// @param	maxDepth	Maximum depth of the trees (default
			///						ceil(log2(#labels))
			//------------------------------------------------------------------
			WeakTree(unsigned int maxDepth = 0);

			//_____ Methods inherited from WeakLearner _______
		public:
			//------------------------------------------------------------------
			/// @brief	Trains the weak learner
			///
			/// @param	inputSet	The input set over which the weak learner
			///						should be trained
			/// @param	costs		Cost matrix (cost of the labels per sample)
			//------------------------------------------------------------------
			void train(InputSet& inputSet,
					   const std::vector<std::vector<double> >& costs);

			//------------------------------------------------------------------
			/// @brief	Classifies the given sample
			///
			/// @param	inputSet	The input set containing the sample to
			///						classify
			/// @param	sample 		The index of the sample to classify
			/// @return				The label of the predicted class
			//------------------------------------------------------------------
			unsigned int classify(InputSet& inputSet,
								  unsigned int sample) const;

			//------------------------------------------------------------------
			/// @brief	Outputs the weak learner to a stream
			///
			/// @param	out	The output stream
			//------------------------------------------------------------------
			void print(std::ostream& out) const;

			//------------------------------------------------------------------
			/// @brief	Reports the features used by the weak learner
			///
			/// @retval	features	The vector of features to populate
			//------------------------------------------------------------------
			void report(std::vector<unsigned int>& features) const;

			//------------------------------------------------------------------
			/// @brief	Remaps the features used internally by the weak learner
			///
			/// @param	mapping	The map of features to use for the mapping
			//------------------------------------------------------------------
			void remap(const std::vector<unsigned int>& mapping);

			//_____ Internal types _______
		private:
			struct Node {
				unsigned int feature_;	///< The feature used for classification
				scalar_t split_;		///< The split point along the feature
				unsigned int labels_[2];///< Predicted labels
			};

			//_____ Internal methods _______
		private:
			// Used to construct the tree recursively
			bool train(const InputSet& inputSet,
					   const std::vector<std::vector<double> >& costs,
					   const std::vector<double>& sums,
					   unsigned int* indices,
					   unsigned int nbSamples,
					   unsigned int depth);

			// Used to print the tree recursively
			void print(std::ostream& out,
					   unsigned int index,
					   unsigned int depth) const;

			//_____ Attributes _______
		private:
			unsigned int maxDepth_;
			std::vector<Node> nodes_;
		};

		//_____ Attributes _______
	private:
		unsigned int nbRounds_;
		unsigned int nbFeaturesPerHeuristic_;
		unsigned int maxDepth_;
		double tca_;
		std::vector<WeakTree> weakLearners_;
		std::vector<scalar_t> alphas_;
		std::vector<unsigned int> reported_;
	};
} // namespace ML

#endif // ML_CLASSIFIERIS_ADABOOST_H

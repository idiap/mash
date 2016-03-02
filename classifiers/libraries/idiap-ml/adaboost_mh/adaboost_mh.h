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


/// \file	adaboost_mh.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 4, 2011

#ifndef ML_ADABOOST_MH_H
#define ML_ADABOOST_MH_H

#include "adaboost_mh_weaktree.h"

namespace ml {
	/// \brief	Discrete AdaBoost.MH algorithm
	class AdaBoostMH : public IClassifier {
	public:
		/// \brief	Constructor
		/// \param	nbRounds				The number of desired boosting
		///									rounds (default 100)
		/// \param	nbFeaturesPerHeuristic	The number of features to sample at
		///									each round from each heuristic
		///									(0 means all, default 10)
		/// \param	maxDepth				Maximum depth of the trees (default
		///									1)
		AdaBoostMH(unsigned int nbRounds = 100,
				   unsigned int nbFeaturesPerHeuristic = 10,
				   unsigned int maxDepth = 1);

		/// \brief	Clone method
		/// \return A deep copy of the classifier
		virtual IClassifier* clone() const;

		/// \brief	Trains the classifier
		/// \param	dataset			The data-set over which the classifier
		///							should be trained
		/// \remark	If trained multiple times, a classifier should assume that
		///			the samples are the same but that the features are new
		virtual void train(const IDataSet& dataset);

		/// \brief	Predicts the class memberships for a given sample
		/// \param	dataset The data-set containing the sample to classify
		/// \param	sample	The index of the sample to classify
		/// \retval distr	The predicted class memberships
		/// \remark A classifier MUST implement either this method or %classify
		virtual void distribution(const IDataSet& dataset,
								  unsigned int sample,
								  double* distr) const;

		/// \brief	Reports the features used by the classifier
		/// \param[out]	features	The vector of features to populate
		virtual void report(std::vector<unsigned int>& features) const;

		/// \brief	Loads a previously saved classifier from a json::Value
		/// \param[in]	value	The input json::Value
		virtual void load(const json::Value& value);

		/// \brief	Saves the classifier into a json::Value
		/// \param[out]	value	The output json::Value
		virtual void save(json::Value& value) const;

		/// \brief	Outputs the classifier to a stream in a human readable form
		/// \param[out]	out The output stream
		virtual void print(std::ostream& out) const;

		/// \brief	Returns the last training error
		double trainError() const;

	private:
		unsigned int nbRounds_;
		unsigned int nbFeaturesPerHeuristic_;
		unsigned int maxDepth_;
		double loss_;
		double trainError_;
		std::vector<WeakTreeMH> weakLearners_;
		std::vector<double> alphas_;
		std::vector<unsigned int> reported_;
		std::vector<std::vector<double> > hypotheses_;
	};
} // namespace ml

#endif // ML_ADABOOST_MH_H

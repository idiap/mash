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


/// \file	naive_bayes.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 21, 2011

#ifndef ML_NAIVE_BAYES_H
#define ML_NAIVE_BAYES_H

#include <idiap-ml/classifier_interface.h>

namespace ml {
	/// \brief	Naive Baye's classifier. Assumes that the features are
	/// independent and can be well approximated by Gaussians
	class NaiveBayes : public IClassifier {
	public:
		/// \brief	Constructor
		/// \param	threshold	Divided by the number of labels, it represents
		///						the fraction of the total standard deviation a
		///						feature must reach for every label in order to
		///						be selected (default 0.5)
		NaiveBayes(double threshold = 0.5);

		/// \brief	Clone method
		/// \return A deep copy of the classifier
		virtual IClassifier* clone() const;

		/// \brief	Trains the classifier
		/// \param	dataset			The data-set over which the classifier
		///							should be trained
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

	private:
		double threshold_; ///< Explained in the constructor
		std::vector<double> priors_; ///< The probability prior of every label
		std::vector<double> weights_; ///< The weight of every feature
		std::vector<double> means_; ///< The means of every feature
		std::vector<double> stds_; ///< The std dev. of every feature
		std::vector<unsigned int> reported_; ///< The features used
	};
} // namespace ml

#endif // ML_NAIVE_BAYES_H

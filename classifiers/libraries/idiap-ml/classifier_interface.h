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


/// \file	classifier_interface.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 19, 2011

#ifndef ML_CLASSIFIER_INTERFACE_H
#define ML_CLASSIFIER_INTERFACE_H

#include <idiap-ml/dataset/dataset_interface.h>
#include <idiap-ml/json/json.h>

#include <iosfwd>
#include <vector>

namespace ml {
	/// \brief	The base interface that all classifiers must re-define
	class IClassifier {
	public:
		/// \brief	Destructor
		virtual ~IClassifier() {};

		/// \brief	Clone method
		/// \return A deep copy of the classifier
		virtual IClassifier* clone() const = 0;

		/// \brief	Trains the classifier
		/// \param	dataset	The data-set over which the classifier should be
		///					trained
		/// \remark	It is up to a classifier to decide what to do if called
		///			multiple times
		virtual void train(const IDataSet& dataset) = 0;

		/// \brief	Classifies the given sample
		/// \param	dataset The data-set containing the sample to classify
		/// \param	sample	The index of the sample to classify
		/// \return			The label of the predicted class
		/// \remark A classifier MUST implement either this method or
		///			%distribution
		virtual unsigned int classify(const IDataSet& dataset,
									  unsigned int sample) const;

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
		virtual void report(std::vector<unsigned int>& features) const = 0;

		/// \brief	Loads a previously saved classifier from a json::Value
		/// \param[in]	value	The input json::Value
		/// \remark It is optional but recommended for a classifier to implement
		///			the method (as it throw an exception by default)
		virtual void load(const json::Value& value);

		/// \brief	Saves the classifier into a json::Value
		/// \param[out]	value	The output json::Value
		/// \remark It is optional but recommended for a classifier to implement
		///			the method (as it throw an exception by default)
		virtual void save(json::Value& value) const;

		/// \brief	Outputs the classifier to a stream in a human readable form
		/// \param[out]	out The output stream
		/// \remark It is optional but recommended for a classifier to implement
		///			the method (as it prints nothing by default)
		virtual void print(std::ostream& out) const;
	};
} // namespace ml

#endif // ML_CLASSIFIER_INTERFACE_H

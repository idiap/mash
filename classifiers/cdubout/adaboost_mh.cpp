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
/// \file	adaboost_mh.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 5, 2011
///
/// Discrete AdaBoost.MH algorithm
///
/// Valid parameters are:
/// - MAX_NEGATIVES		    The maximum number of negatives to sample from each
///						    image (only if doing detection, default 10)
/// - ADDITIONAL_HEURISTICS	The list of heuristics already in the loaded model
///                         to use during model adaptation (in addition to all
///                         the ones not in the model)
/// - SAVE_TO_DISK			Whether or not to first save the entire dataset to
///							the disk before training (default 0)
/// - NB_ROUNDS			    The number of desired Boosting rounds (default 100)
/// - NB_FEATURES		    The number of features to sample at each round from
///						    each heuristic (0 means all, default 10)
/// - MAX_DEPTH			    Maximum depth of the trees (default 1, decision
///                         stumps)
//------------------------------------------------------------------------------

#include <idiap-ml/dataset/mash_dataset.h>
#include <idiap-ml/dataset/file_dataset.h>
#include <idiap-ml/dataset/dataset_subset.h>
#include <idiap-ml/adaboost_mh/adaboost_mh.h>

#include <iostream>
#include <set>
#include <sstream>
#include <exception>

using namespace Mash;

//------------------------------------------------------------------------------
// Declaration of the 'AdaBoostMH' classifier class
//------------------------------------------------------------------------------
class AdaBoostMH : public Classifier {
	//_____ Construction / Destruction __________
public:
	AdaBoostMH();
	virtual ~AdaBoostMH();

	//_____ Methods to implement __________
public:
	//--------------------------------------------------------------------------
	/// @brief	Initialize the classifier
	///
	/// @param	parameters	The classifier-specific parameters
	/// @return				'true' if successful
	//--------------------------------------------------------------------------
	virtual bool setup(const tExperimentParametersList& parameters);

	//--------------------------------------------------------------------------
	/// @brief	Load a model
	///
	/// @param	model			The model object to use
	/// @param	internal_data	(Optional) Used to retrieve classifier-specific
	///							data saved alongside the model
	/// @return					'true' if successful
	///
	/// @note	This method will be called after setup(), but only if an
	///			already trained model must be used. Then, either we begin
	///			directly with calls to classify(), or the classifier is given
	///			the opportunity to adapt its model via a call to train().
	///
	/// The provided model has already been initialized (ie. it already
	/// knows about the heuristics used by the classifier that saved it).
	///
	/// In some specific scenarios, when saving a model, the classifier can
	/// also save additional data. The goal here is to speed-up subsequent
	/// experiments using the saved model. However, this internal data might
	/// not always be provided alongside the model, so the classifier must
	/// be able to setup itself using only the model (ie. if
	/// internal_data.isOpen() == false, the internal data isn't available).
	//--------------------------------------------------------------------------
	virtual bool loadModel(PredictorModel &model, DataReader &internal_data);

	//--------------------------------------------------------------------------
	/// @brief	Train the classifier
	///
	/// @param		input_set	The Classifier Input Set to use
	/// @param[out] train_error (Optional) The train error. Leave unchanged
	///							if you want to let the Framework compute it.
	/// @return					'true' if successful
	//--------------------------------------------------------------------------
	virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error);

	//--------------------------------------------------------------------------
	/// @brief	Classify the content of an image at a specific position
	///
	/// @param	input_set	The Classifier Input Set to use
	/// @param	image		Index of the image (in the Input Set)
	/// @param	position	Center of the region-of-interest (in the image)
	/// @retval results		Classification results
	/// @return				'true' if successful
	//--------------------------------------------------------------------------
	virtual bool classify(IClassifierInputSet* input_set,
						  unsigned int image,
						  const coordinates_t& position,
						  tClassificationResults &results);

	//--------------------------------------------------------------------------
	/// @brief	Populates the provided list with the features used by the
	///			classifier
	///
	/// @retval list	The list of features used
	/// @return			'true' if successful
	///
	/// @note	This method will only be called after train(), and the 'list'

	///			parameter will always be empty.
	//--------------------------------------------------------------------------
	virtual bool reportFeaturesUsed(tFeatureList &list);

	//--------------------------------------------------------------------------
	/// @brief	Save the model trained by the classifier
	///
	/// @param	model	The model object to use
	/// @return			'true' if successful
	///
	/// @note	This method will only be called at the end of the experiment.
	///
	/// The provided model has already been initialized (ie. it already
	/// knows about the heuristics used by the classifier, reported by
	/// reportFeaturesUsed()).
	///
	/// In some specific scenarios, the classifier can save whatever it wants
	/// using the attribute 'outInternalData'. The goal here is to speed-up
	/// subsequent experiments using the saved model. However, this internal
	/// data might not always be provided alongside the model, so the
	/// classifier must be able to setup itself using only the model.
	//--------------------------------------------------------------------------
	virtual bool saveModel(PredictorModel &model);

	//_____ Protected methods __________
protected:
	//--------------------------------------------------------------------------
	/// @brief	Load a model from the internal model string
	//--------------------------------------------------------------------------
	virtual bool loadModel(json::Value& value);

	//_____ Attributes __________
protected:
	// Dataset related
	ml::MashDataSet* dataset_; /// The current Mash input set wrapper
	unsigned int id_; ///< The id of the current Mash input set
	unsigned int maxNegatives_; ///< The number of negatives per image
	std::set<std::string> newHeuristics_; ///< To use for training
	bool saveToDisk_; ///< Whether or not to first save the features to the disk

	// Classifier related
	ml::AdaBoostMH* classifier_; ///< The classifier to train and test
	PredictorModel model_; ///< Copy of the predictor model
	DataReader internal_data_; ///< Copy of the internal data
	bool needLoading_; ///< True if there is a model waiting to be loaded
};

//------------------------------------------------------------------------------
// Creation function of the classifier
//------------------------------------------------------------------------------
extern "C" Classifier* new_classifier() {
	return new AdaBoostMH();
}

//------------------------------------------------------------------------------
// Custom streambuf used to redirect cout to the classifier's outStream
//------------------------------------------------------------------------------
class StreamBuf : public std::streambuf {
public:
	StreamBuf(Mash::OutStream& outStream);
	~StreamBuf();

protected:
	virtual std::streamsize xsputn(const char* s, std::streamsize n);
	virtual int overflow(int c = EOF);

private:
	Mash::OutStream& outStream_; ///< The stream to use for logging
	std::streambuf* cout_; ///< The original streambuf object of std::cout
	bool recursive_; ///< Indicate if the call originated from inside
};

StreamBuf::StreamBuf(Mash::OutStream& outStream)
: outStream_(outStream), cout_(std::cout.rdbuf()), recursive_(false) {
	std::cout.rdbuf(this);
}

StreamBuf::~StreamBuf() {
	std::cout.rdbuf(cout_);
}

std::streamsize StreamBuf::xsputn(const char* s, std::streamsize n) {
	if (recursive_) {
		return cout_->sputn(s, n);
	}

	if (s && n) {
		recursive_ = true;
		outStream_.write(s, n);
		recursive_ = false;
	}

	return n;
}

int StreamBuf::overflow(int c) {
	if (recursive_) {
		return cout_->sputc(c);
	}

	if (c != EOF) {
		recursive_ = true;
		outStream_.write(reinterpret_cast<char*>(&c), 1);
		recursive_ = false;
	}

	return 1;
}

/************************* CONSTRUCTION / DESTRUCTION *************************/

AdaBoostMH::AdaBoostMH() {
	// Dataset related
	dataset_ = 0;
	id_ = 0;
	maxNegatives_ = 10;
	saveToDisk_ = false;

	// Classifier related
	classifier_ = 0;
	needLoading_ = false;
}

AdaBoostMH::~AdaBoostMH() {
	delete dataset_;
	delete classifier_;
}

/************************* IMPLEMENTATION OF Classifier ***********************/

bool AdaBoostMH::setup(const tExperimentParametersList& parameters) {
	// AdaBoost.MH parameters
	unsigned int nbRounds = 100;
	unsigned int nbFeatures = 10;
	unsigned int maxDepth = 1;

	// Parse all parameters
	for (tExperimentParametersIterator iter = parameters.begin();
		 iter != parameters.end(); ++iter) {
		if (iter->first == "MAX_NEGATIVES") {
			maxNegatives_ = iter->second.getInt(0);
		}
		else if (iter->first == "ADDITIONAL_HEURISTICS") {
			for (int i = 0; i < iter->second.size(); ++i) {
				newHeuristics_.insert(iter->second.getString(i));
			}
		}
		else if (iter->first == "SAVE_TO_DISK") {
			saveToDisk_ = iter->second.getInt(0);
		}
		else if (iter->first == "NB_ROUNDS") {
			nbRounds = iter->second.getInt(0);
		}
		else if (iter->first == "NB_FEATURES") {
			nbFeatures = iter->second.getInt(0);
		}
		else if (iter->first == "MAX_DEPTH") {
			maxDepth = iter->second.getInt(0);
		}
		else {
			outStream << "Warning: unknown parameter '" << iter->first << "'."
					  << std::endl;
		}
	}

	try {
		// Redirect std::cout to the classifier's outStream
		StreamBuf cout(outStream);

		classifier_ = new ml::AdaBoostMH(nbRounds, nbFeatures, maxDepth);
	}
	catch (std::exception& e) {
		outStream << "Exception raised in setup: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool AdaBoostMH::loadModel(PredictorModel &model, DataReader &internal_data) {
	// Copy the predictor model
	model_ = model;
	needLoading_ = true;
	return true;
}

bool AdaBoostMH::train(IClassifierInputSet* input_set, scalar_t &train_error) {
	if (dataset_) {
		outStream << "Attempting to train multiple times, or to train after "
					 "calling classify" << std::endl;
		return false;
	}

	try {
		// Redirect std::cout to the classifier's outStream
		StreamBuf cout(outStream);

		// Create an ml::MashDataSet from the Mash::IClassifierInputSet
		dataset_ = new ml::MashDataSet(input_set, maxNegatives_);
		id_ = input_set->id();

		// Load the model if needed
		if (needLoading_) {
			// Add all the heuristics not in the model
			for (unsigned int h = 0; h < input_set->nbHeuristics(); ++h) {
				if (!input_set->isHeuristicUsedByModel(h) &&
				    !newHeuristics_.count(input_set->heuristicName(h))) {
					newHeuristics_.insert(input_set->heuristicName(h));
				}
			}

			// It is an error if there is no new heuristic
			if (newHeuristics_.empty()) {
				outStream << "Attempting to retrain a classifier without any "
							 "new heuristic" << std::endl;
				return false;
			}

			// Load the classifier
			json::Value value;

			if (!loadModel(value)) {
				return false;
			}

			// Recopy only the necessary fields
			json::Value tmpValue;
			tmpValue["name"] = value["name"];
			tmpValue["loss"] = value["loss"];
			tmpValue["trainError"] = value["trainError"];
			tmpValue["weakLearners"].array();
			tmpValue["alphas"].array();
			tmpValue["reported"].array();
			tmpValue["hypotheses"] = value["hypotheses"];
			classifier_->load(tmpValue);

			// Select only the new heuristics
			tFeatureList list;
			std::vector<unsigned int> features;

			// Add all the new heuristics
			outStream << "New heuristic(s) found:";

			for (unsigned int h = 0; h < input_set->nbHeuristics(); ++h) {
				if (newHeuristics_.count(input_set->heuristicName(h))) {
					outStream << " " << input_set->heuristicName(h);

					for (unsigned int f = 0; f < input_set->nbFeatures(h); ++f) {
						list.push_back(tFeature(h, f));
					}
				}
			}

			outStream << std::endl;

			dataset_->convert(list, features);

			// Train the classifier on the new heuristics
			ml::FeatureSubSet subset(dataset_, features.size(), &features[0]);
			classifier_->train(subset);
			train_error = classifier_->trainError();

			// Print the classifier to the log
			std::ostringstream oss;
			classifier_->print(oss);
			outStream << oss.str();

			// Merge the classifiers
			classifier_->save(tmpValue);
			value["loss"] = tmpValue["loss"];
			value["trainError"] = tmpValue["trainError"];

			const unsigned int offset = value["reported"].array().size();

			for (unsigned int w = 0; w < tmpValue["weakLearners"].array().size(); ++w) {
				json::Value& weakLearner = tmpValue["weakLearners"][w];

				for (unsigned int n = 0; n < weakLearner["nodes"].array().size(); ++n) {
					weakLearner["nodes"][n]["feature"] += offset;
				}

				value["weakLearners"].array().push_back(weakLearner);
				value["alphas"].array().push_back(tmpValue["alphas"][w]);
			}

			for (unsigned int f = 0; f < tmpValue["reported"].array().size(); ++f) {
				value["reported"].array().push_back(features[tmpValue["reported"][f]]);
			}

			value["hypotheses"].array().swap(tmpValue["hypotheses"].array());

			classifier_->load(value);

			needLoading_ = false;
		}
		else {
			// It is an error if there is new heuristic
			if (!newHeuristics_.empty()) {
				outStream << "Attempting to train a classifier without all the "
							 "heuristics" << std::endl;
				return false;
			}

			// Use a file dataset to speed up the training
			if (saveToDisk_) {
				ml::FileDataSet::save(*dataset_, outFeatureCache);
				ml::FileDataSet dataset(inFeatureCache);

				// Train the classifier
				classifier_->train(dataset);
			}
			else {
				classifier_->train(*dataset_);
			}

			train_error = classifier_->trainError();

			// Print the classifier to the log
		//	std::ostringstream oss;
		//	classifier_->print(oss);
		//	outStream << oss.str();
		}
	}
	catch (std::exception& e) {
		outStream << "Exception raised in train: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool AdaBoostMH::classify(IClassifierInputSet* input_set,
						  unsigned int image,
						  const coordinates_t& position,
						  tClassificationResults &results) {
	// Redirect std::cout to the classifier's outStream
	StreamBuf cout(outStream);

	// Create an ml::MashDataSet if there is none or if the Mash input changed
	if (!dataset_ || id_ != input_set->id()) {
		delete dataset_;
		dataset_ = new ml::MashDataSet(input_set, 0);
		id_ = input_set->id();
	}

	// Load the model if needed
	if (needLoading_) {
		json::Value value;

		if (!loadModel(value)) {
			return false;
		}

		try {
			classifier_->load(value);
		}
		catch (std::exception& e) {
			outStream << "Exception raised in classify: " << e.what() << std::endl;
			return false;
		}

		needLoading_ = false;
	}

	// Add the sample to the dataset
	dataset_->pushSample(0, image, position);

	// Computes the distribution of memberships
	std::vector<double> distr(dataset_->nbLabels());

	try {
		classifier_->distribution(*dataset_, dataset_->nbSamples() - 1, &distr[0]);
	}
	catch (std::exception& e) {
		outStream << "Exception raised in classify: " << e.what() << std::endl;
		return false;
	}

	// Remove the sample
	dataset_->popSample();

	// Recopy the distribution into results
	if (distr.size() <= input_set->nbLabels()) {
		for (unsigned int l = 0; l < distr.size(); ++l) {
			results[l] = distr[l];
		}
	}
	else {
		for (unsigned int l = 0; l < input_set->nbLabels(); ++l) {
			results[l] = distr[l] - distr.back();
		}
	}

	// Save the classification result
	outInternalData << input_set->isImageInTestSet(image) << " " << image << " "
					<< position.x << " " << position.y << " " << distr.size();

	for (unsigned int l = 0; l < distr.size(); ++l) {
		outInternalData << " " << distr[l];
	}

	outInternalData << std::endl;

	return true;
}

bool AdaBoostMH::reportFeaturesUsed(tFeatureList &list) {
	// There must be a classifier and a dataset to convert features
	if (!classifier_ || !dataset_) {
		return false;
	}

	std::vector<unsigned int> reported;
	classifier_->report(reported);
	dataset_->convert(reported, list);

	return true;
}

bool AdaBoostMH::saveModel(PredictorModel &model) {
	// There must be a classifier and a dataset to convert features
	if (!classifier_ || !dataset_) {
		return false;
	}

	// Save the classifier in a json::Value
	json::Value value;

	try {
		classifier_->save(value);
	}
	catch (std::exception& e) {
		outStream << "Exception raised in saveModel: " << e.what() << std::endl;
		return false;
	}

	// Convert the reported features to model space
	std::vector<unsigned int> reported(value["reported"].array().begin(),
									   value["reported"].array().end());

	// First convert the features to Mash features
	tFeatureList list;
	dataset_->convert(reported, list);

	// Then change the heuristic id and write it in the json::Value
	reported.clear();

	for (unsigned int f = 0; f < list.size(); ++f) {
		reported.push_back(model.toModel(list[f].heuristic));
		reported.push_back(list[f].feature_index);
	}

	// Replace the array of reported features
	value["reported"].array().assign(reported.begin(), reported.end());

	// Save the json::Value in a string stream
	std::ostringstream oss;
	value.write(oss);
	model.writer() << oss.str();

	return true;
}

bool AdaBoostMH::loadModel(json::Value& value) {
	// There must be a classifier and a dataset to convert features
	if (!classifier_ || !dataset_) {
		return false;
	}

	// Determine the length of the model
	model_.reader().seek(0, DataReader::END);
	int64_t length = model_.reader().tell();
	model_.reader().seek(0, DataReader::BEGIN);

	// Read the whole model into a string
	std::string model(length, '\0');
	model_.reader().read(reinterpret_cast<int8_t*>(&model[0]), length);

	// Parse the string into a json::Value
	try {
		std::istringstream iss(model);
		value.parse(iss);
	}
	catch (std::exception& e) {
		outStream << "Exception raised in loadModel: " << e.what() << std::endl;
		return false;
	}

	// Convert the reported features from model space
	std::vector<unsigned int> reported(value["reported"].array().begin(),
									   value["reported"].array().end());

	// Convert to Mash::tFeature's
	tFeatureList list;

	for (unsigned int f = 0; f < reported.size(); f += 2) {
		list.push_back(tFeature(model_.fromModel(reported[f]), reported[f + 1]));
	}

	// Convert from Mash::tFeature's
	reported.clear();
	dataset_->convert(list, reported);

	// Replace the array of reported features
	value["reported"].array().assign(reported.begin(), reported.end());

	return true;
}

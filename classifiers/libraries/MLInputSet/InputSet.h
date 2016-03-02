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
/// @file MLInputSet/InputSet.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.08
/// @version 2.1
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_INPUTSET_H
#define ML_INPUTSET_INPUTSET_H

#include <mash-classification/classifier.h>

#include "Cache.h"

namespace ML {
	// Import the required types from the Mash namespace
	using Mash::scalar_t;
	using Mash::coordinates_t;

	// Maximum number of features authorized to have in memory (512MB)
	static const unsigned int NB_FEATURES_MAX = (1 << 29) / sizeof(scalar_t);

	//--------------------------------------------------------------------------
	/// @brief	IClassifierInputSet abstract wrapper class
	///
	/// Simplify the implementation of classifiers:
	/// - Classifiers have to deal only with samples/features (no image or
	///   heuristic stuff, although they can still know the label, image, and
	///   coordinates of every sample and the heuristic of every feature)
	/// - They can transparently select a subset of samples and features,
	///   modify the labels, assign weights to the samples, etc.
	/// - The input set supports different kind of sample/feature/heuristic
	///   selection and normalization through external filters
	/// - It handles object detection easily (simply add an extra label for the
	///   negatives)
	//--------------------------------------------------------------------------
	class InputSet {
		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor
		//----------------------------------------------------------------------
		InputSet();

		//----------------------------------------------------------------------
		/// @brief	Destructor
		///
		/// Virtual destructor so as to make sure that the destructors of
		/// derived classes will also get called.
		//----------------------------------------------------------------------
		virtual ~InputSet();

		//_____ Public methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Returns the number of samples
		//----------------------------------------------------------------------
		unsigned int nbSamples() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the number of features
		//----------------------------------------------------------------------
		unsigned int nbFeatures() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the number of distinct labels
		//----------------------------------------------------------------------
		unsigned int nbLabels() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the number of distinct images
		//----------------------------------------------------------------------
		unsigned int nbImages() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the number of distinct heuristics
		//----------------------------------------------------------------------
		unsigned int nbHeuristics() const;

		//----------------------------------------------------------------------
		/// @brief	Pushes the indexed samples on the sample stack
		///
		/// @param	indices	Vector of indices of selected samples
		//----------------------------------------------------------------------
		void pushSamples(const std::vector<unsigned int>& indices);

		//----------------------------------------------------------------------
		/// @brief	Pops the last pushed samples from the sample stack
		//----------------------------------------------------------------------
		void popSamples();

		//----------------------------------------------------------------------
		/// @brief	Pushes the indexed features on the feature stack
		///
		/// @param	indices	Vector of indices of selected features
		//----------------------------------------------------------------------
		void pushFeatures(const std::vector<unsigned int>& indices);

		//----------------------------------------------------------------------
		/// @brief	Pops the last pushed features from the feature stack
		//----------------------------------------------------------------------
		void popFeatures();

		//----------------------------------------------------------------------
		/// @brief	Returns the values of the features associated with a sample
		///
		/// @remark	Can return the full matrix of features as a
		///			nbSamples x nbFeatures matrix, if called with no argument
		//----------------------------------------------------------------------
		const scalar_t* features(unsigned int sample = 0) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the values for every sample of a particular feature
		///
		/// @remark	Can return the full matrix of features as a
		///			nbFeatures x nbSamples matrix, if called with no argument
		//----------------------------------------------------------------------
		const scalar_t* samples(unsigned int feature = 0) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the labels of the samples
		///
		/// @remark	All the labels lie in the range [0, nbLabels)
		//----------------------------------------------------------------------
		const unsigned int* labels() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the label of a particular sample
		///
		/// @remark	Really just a shortcut to %labels()[sample]
		//----------------------------------------------------------------------
		unsigned int label(unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the weights of the samples
		///
		/// @remark	The weights are non-negative
		//----------------------------------------------------------------------
		const scalar_t* weights() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the weight of a particular sample
		///
		/// @remark	Really just a shortcut to %weights()[sample]
		//----------------------------------------------------------------------
		scalar_t weight(unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the images of the samples
		///
		/// @remark	All the images lie in the range [0, nbImages)
		//----------------------------------------------------------------------
		const unsigned int* images() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the image of a particular sample
		///
		/// @remark	Really just a shortcut to %images()[sample]
		//----------------------------------------------------------------------
		unsigned int image(unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the coordinates of the samples
		//----------------------------------------------------------------------
		const coordinates_t* coordinates() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the coordinates of a particular sample
		///
		/// @remark	Really just a shortcut to %coordinates()[sample]
		//----------------------------------------------------------------------
		coordinates_t coordinates(unsigned int sample) const;

		//----------------------------------------------------------------------
		/// @brief	Returns the heuristics of the features
		///
		/// @remark	All the heuristics lie in the range [0, nbHeuristics)
		//----------------------------------------------------------------------
		const unsigned int* heuristics() const;

		//----------------------------------------------------------------------
		/// @brief	Returns the heuristic of a particular feature
		///
		/// @remark	Really just a shortcut to %heuristics()[feature]
		//----------------------------------------------------------------------
		unsigned int heuristic(unsigned int feature) const;

		//_____ Public (advanced) methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Swaps the features with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					matrix of features (nbSamples x nbFeatures)
		///	@param	flag	Whether or not to compute the matrix if it is not
		///					already in the cache (default true, can be set to
		///					false if the caller only cares about setting the
		///					matrix)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Very useful to classifiers that require to save the training
		///			matrix of features such as k-NN or SVM
		//----------------------------------------------------------------------
		void swapFeatures(std::vector<scalar_t>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the features with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					matrix of features (nbFeatures x nbSamples)
		///	@param	flag	Whether or not to compute the matrix if it is not
		///					already in the cache (default true, can be set to
		///					false if the caller only cares about setting the
		///					matrix)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Very useful to classifiers that require to save the training
		///			matrix of features such as k-NN or SVM
		//----------------------------------------------------------------------
		void swapSamples(std::vector<scalar_t>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the label vector with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					vector of labels
		///	@param	flag	Whether or not to compute the vector of labels if it
		///					is not already in the cache (default true, can be
		///					set to false if the caller only cares about setting
		///					the labels)
		///
		/// @remark	Sets the number of labels to max(data) + 1 (the previous
		///			number is restored when the cache disapear)
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Can be useful to classifiers that require to modify the
		///			labels such as a product of base classifier
		//----------------------------------------------------------------------
		void swapLabels(std::vector<unsigned int>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the weight vector with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					vector of weights
		///	@param	flag	Whether or not to compute the vector of weights if
		///					it is not already in the cache (default true, can be
		///					set to false if the caller only cares about setting
		///					the weights)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Useful to classifiers that require to modify the weights
		///			such as AdaBoost
		//----------------------------------------------------------------------
		void swapWeights(std::vector<scalar_t>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the image vector with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					vector of images
		///	@param	flag	Whether or not to compute the vector of images if it
		///					is not already in the cache (default true, can be
		///					set to false if the caller only cares about setting
		///					the images)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Useful when computing the train or test error as it allows
		///			the creation of new virtual samples by modifying the image
		///			of an existing sample
		//----------------------------------------------------------------------
		void swapImages(std::vector<unsigned int>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the coordinates vector with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					vector of coordinates
		///	@param	flag	Whether or not to compute the vector of coordinates
		///					if it is not already in the cache (default true, can
		///					be set to false if the caller only cares about
		///					setting the coordinates)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Useful when computing the train or test error as it allows
		///			the creation of new virtual samples by modifying the
		///			coordinates of an existing sample
		//----------------------------------------------------------------------
		void swapCoordinates(std::vector<coordinates_t>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Swaps the heuristic vector with an std::vector
		///
		/// @param	data	The std::vector with which to swap the current
		///					vector of heuristics
		///	@param	flag	Whether or not to compute the vector of heuristics
		///					if it is not already in the cache (default true, can
		///					be set to false if the caller only cares about
		///					setting the heuristics)
		///
		/// @remark	Swapping with an empty vector delete the current cache
		/// @remark	Not sure for what it could be useful, but here for
		///			consistency
		//----------------------------------------------------------------------
		void swapHeuristics(std::vector<unsigned int>& data, bool flag = true);

		//----------------------------------------------------------------------
		/// @brief	Clear all the caches completely
		///
		/// @remark	Virtual so that derived input set can also clear their
		///			own attributes
		//----------------------------------------------------------------------
		virtual void clear();

		//----------------------------------------------------------------------
		/// @brief	Samples features fairly from every heuristic
		///
		/// @param	nbTotal	Total number of features to sample (default,
		///					maximum number of features that can fit in memory)
		/// @retval	indices	The indices of the selected features
		//----------------------------------------------------------------------
		void sampleFeatures(unsigned int nbTotal,
							std::vector<unsigned int>& indices);

		//_____ Methods to implement _______
	protected:
		//----------------------------------------------------------------------
		/// @brief	Creates the first matrix cache
		//----------------------------------------------------------------------
		virtual void matrixCache(std::vector<scalar_t>& data) const = 0;

		//----------------------------------------------------------------------
		/// @brief	Creates the first label cache
		//----------------------------------------------------------------------
		virtual void labelCache(std::vector<unsigned int>& data) const = 0;

		//----------------------------------------------------------------------
		/// @brief	Creates the first weight cache
		//----------------------------------------------------------------------
		virtual void weightCache(std::vector<scalar_t>& data) const = 0;

		//----------------------------------------------------------------------
		/// @brief	Creates the first image cache
		//----------------------------------------------------------------------
		virtual void imageCache(std::vector<unsigned int>& data) const = 0;

		//----------------------------------------------------------------------
		/// @brief	Creates the first coordinates cache
		//----------------------------------------------------------------------
		virtual void coordinatesCache(std::vector<coordinates_t>& data) const = 0;

		//----------------------------------------------------------------------
		/// @brief	Creates the first heuristic cache
		//----------------------------------------------------------------------
		virtual void heuristicCache(std::vector<unsigned int>& data) const = 0;

		//_____ Attributes _______
	protected:
		/// The sample stack
		std::vector<std::vector<unsigned int> > sampleStack_;

		/// The feature stack
		std::vector<std::vector<unsigned int> > featureStack_;

		/// The number of labels stack
		std::vector<unsigned int> nbLabelStack_;

		unsigned int nbImages_;		///< The number of images
		unsigned int nbHeuristics_;	///< The number of heuristics

		// The caches
		mutable Cache2D<scalar_t> matrixCache_;
		mutable Cache1D<unsigned int> labelCache_;
		mutable Cache1D<scalar_t> weightCache_;
		mutable Cache1D<unsigned int> imageCache_;
		mutable Cache1D<coordinates_t> coordinatesCache_;
		mutable Cache1D<unsigned int> heuristicCache_;
	};

	//_____ Inline methods _______
	inline unsigned int InputSet::label(unsigned int sample) const {
		return labels()[sample];
	}

	inline scalar_t InputSet::weight(unsigned int sample) const {
		return weights()[sample];
	}

	inline unsigned int InputSet::image(unsigned int sample) const {
		return images()[sample];
	}

	inline coordinates_t InputSet::coordinates(unsigned int sample) const {
		return coordinates()[sample];
	}

	inline unsigned int InputSet::heuristic(unsigned int feature) const {
		return heuristics()[feature];
	}
} // namespace ML

#endif // ML_INPUTSET_INPUTSET_H

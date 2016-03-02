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
/// @file MLClassifiers/Utils.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.01.25
/// @version 1.7
//------------------------------------------------------------------------------

#ifndef ML_CLASSIFIERS_UTILS_H
#define ML_CLASSIFIERS_UTILS_H

#include "Classifier.h"

#include <cmath>
// #include <stdint.h>

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	Class implementing some simple utility methods
	//--------------------------------------------------------------------------
	class Utils {
		//_____ Public constants _______
	public:
		/// The small deviation allowed in scalar comparisons
		static const scalar_t epsilon;

		/// The '+infinity' ('-infinity' can be obtained by simply adding a '-')
		static const scalar_t infinity;

		//_____ Public methods _______
	public:
		/// Tests if @p a is equal to @p b
		static bool eq(scalar_t a, scalar_t b);

		/// Tests if @p a is different from @p b
		static bool neq(scalar_t a, scalar_t b);

		/// Tests if @p a is strictly less than @p b
		static bool less(scalar_t a, scalar_t b);

		/// Tests if @p a is strictly greater than @p b
		static bool greater(scalar_t a, scalar_t b);

		/// Tests if @p a is less or equal to @p b
		static bool leq(scalar_t a, scalar_t b);

		/// Tests if @p a is greater or equal to @p b
		static bool geq(scalar_t a, scalar_t b);

		/// Returns the logarithm of @p x base 2
		static scalar_t log2(scalar_t x);

		/// Returns the entropy associated with a probability @p p
		static scalar_t entropy(scalar_t p);

		//----------------------------------------------------------------------
		/// @brief	Sorts in place a vector of indices based on the values that
		///			they index
		///
		/// @param	indices		Array of @p nbSamples indices to sort
		/// @param	values		Array of @p nbSamples values
		/// @param	nbSamples	The number of samples
		//----------------------------------------------------------------------
		static void sort(unsigned int* indices,
						 const scalar_t* values,
						 unsigned int nbSamples);

		//----------------------------------------------------------------------
		/// @brief	Partitions in place a vector of indices depending if the
		///			value that they index is lower or equal (<=) or greater (>)
		///			than a @p pivot value
		///
		/// @param	indices		Array of @p nbSamples indices to partition
		/// @param	values		Array of @p nbSamples values
 		/// @param	nbSamples	The number of samples
		/// @param	pivot		The pivot value
		/// @return				The position of the first index belonging to the
		///						second partition
		//----------------------------------------------------------------------
		static unsigned int partition(unsigned int* indices,
									  const scalar_t* values,
									  unsigned int nbSamples,
									  scalar_t pivot);

		//----------------------------------------------------------------------
		/// @brief	Samples indices at random according to a weight distribution
		///
		/// @param	weights		Array of %nbWeights weights
		/// @param	nbWeights	The number of weights
		/// @param	indices		Array of %nbIndices indices that the method need
		///						to fill
		/// @param	nbIndices	Number of indices to sample
		//----------------------------------------------------------------------
		static void robustSampling(const scalar_t* weights,
								   unsigned int nbWeights,
								   unsigned int* indices,
								   unsigned int nbIndices);

		//----------------------------------------------------------------------
		/// @brief Finds the abscissa of the minima of the convex function @p f
		/// in the range [x1 x3] using golden section search. Terminates when
		/// within the relative accuracy bounds of the function argument type
		/// See http://en.wikipedia.org/wiki/Golden_section_search for details
		///
		/// @param x1 Lower bound
		/// @param x3 Upper bound
		/// @param f A unary function object
		/// @param eps determines the accuracy of the search
		/// @return the abscissa of the minima
		//----------------------------------------------------------------------
		template<typename Function>
		static scalar_t lineSearch(scalar_t x1,
								   scalar_t x3,
								   Function f,
								   scalar_t eps = epsilon);
	};

	// Based on fcmp 1.2.2 Copyright (c) 1998-2000 Theodore C. Belding
	// University of Michigan Center for the Study of Complex Systems
	// Ted.Belding@umich.edu
	inline int fcmp(scalar_t a, scalar_t b) {
		// Get exponent(max(abs(a), abs(b))) and store it in exponent
		int exponent;
		std::frexp(std::abs(a) > std::abs(b) ? a : b, &exponent);

		// delta = epsilon * pow(2, exponent)
		scalar_t delta = std::ldexp(Utils::epsilon, exponent);

		scalar_t difference = a - b;

		// Return 1 if a > b, -1 if a < b, 0 otherwise
		return (difference > delta) ? 1 : ((difference < -delta) ? -1 : 0);
	}

	inline bool Utils::eq(scalar_t a, scalar_t b) {
		return fcmp(a, b) == 0;
	}

	inline bool Utils::neq(scalar_t a, scalar_t b) {
		return fcmp(a, b) != 0;
	}

	inline bool Utils::less(scalar_t a, scalar_t b) {
		return fcmp(a, b) == -1;
	}

	inline bool Utils::greater(scalar_t a, scalar_t b) {
		return fcmp(a, b) == 1;
	}

	inline bool Utils::leq(scalar_t a, scalar_t b) {
		return fcmp(a, b) != 1;
	}

	inline bool Utils::geq(scalar_t a, scalar_t b) {
		return fcmp(a, b) != -1;
	}

	inline scalar_t Utils::log2(scalar_t x) {
		// Ground truth
		return std::log(x) * 1.4426950408;

		// Quick approximation for float, precision ~1%
		// Warning: returns garbage if x <= 0
	//	int32_t* i = reinterpret_cast<int32_t*>(&x);
	//	const int log2 = (*i >> 23) - 128;
	//	*i &= ~(255 << 23);
	//	*i |=  (127 << 23);
	//	return log2 + ((-1.0f / 3.0f) * x + 2.0f) * x - 2.0f / 3.0f;
	}

	inline scalar_t Utils::entropy(scalar_t p) {
		return (p > 0) ? -p * Utils::log2(p) : 0;
	}

	template<typename Function>
	scalar_t Utils::lineSearch(scalar_t x1,
							   scalar_t x3,
							   Function f,
							   scalar_t eps) {
		const scalar_t C = 0.381966011; // 2 - (1 + sqrt(5)) / 2

		// Make sure that x1 < x3
		if(x3 < x1) {
			scalar_t tmp = x1;
			x1 = x3;
			x3 = tmp;
		}

		// Choose x2 so that (x3 - x2) / (x2 - x1) = golden ratio
		scalar_t x2 = x1 + C * (x3 - x1);

		// Choose x4 so that x4 - x1 = x3 - x2
		scalar_t x4 = x1 - x2 + x3;

		// Calculate f2 and f4
		scalar_t f2 = f(x2);
		scalar_t f4 = f(x4);

		// While within the relative accuracy bounds of argument_type
		while(std::abs(x3 - x1) > eps + eps * (std::abs(x2) + std::abs(x4))) {
			if(f4 < f2) {
				x1 = x2;
				x2 = x4;
				x4 = x2 + C * (x3 - x2);
				f2 = f4;
				f4 = f(x4);
			}
			else {
				x3 = x4;
				x4 = x2;
				x2 = x4 + C * (x1 - x4);
				f4 = f2;
				f2 = f(x2);
			}
		}

		// Return the abscissa of the minimum
		return (f2 < f4) ? x2 : x4;
	}	
} // namespace ML

#endif // ML_CLASSIFIERS_UTILS_H

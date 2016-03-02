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


/// \file	dataset_relabel.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 28, 2011

#ifndef ML_DATASET_RELABEL_H
#define ML_DATASET_RELABEL_H

#include "dataset_filter.h"

#include <vector>

namespace ml {
	/// \brief	Relabel a dataset
	class DataSetRelabel : public DataSetFilter {
	public:
		/// \brief	Constructor
		/// \param	dataset	The dataset to cache
		/// \param	map		The label map
		DataSetRelabel(const IDataSet* dataset,
					   const unsigned int* map);

		/// \brief	Returns (an upper-bound on) the number of distinct labels
		virtual unsigned int nbLabels() const;

		/// \brief	Returns the label of a \param sample
		/// \remark Labels always lie in the range [0, %nbLabels)
		virtual unsigned int label(unsigned int sample) const;

	private:
		std::vector<unsigned int> map_;
	};
} // namespace ml

#endif // ML_DATASET_CACHE_H

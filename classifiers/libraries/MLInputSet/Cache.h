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
/// @file MLInputSet/Cache.hpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.07.26
/// @version 1.0
//------------------------------------------------------------------------------

#ifndef ML_INPUTSET_CACHE_H
#define ML_INPUTSET_CACHE_H

#include "Transpose.h"

#include <cassert>
#include <vector>

namespace ML {
	//--------------------------------------------------------------------------
	/// @brief	A 1D cache (in reality a stack of them)
	///
	/// Caches are stored as stacks of chunks, so that they can be updated to
	/// follow the sample or feature stack(s) (the user can only see the most
	/// recent version of the cache, so that the swap cache methods simply swaps
	/// vectors).
	//--------------------------------------------------------------------------
	template <typename T>
	class Cache1D {
		//_____ Internal types _______
	private:
		//----------------------------------------------------------------------
		/// @brief	A 1D cache chunk
		//----------------------------------------------------------------------
		struct Chunk {
			const unsigned int	level_;	 ///< Level in the stack
			std::vector<T>		data_;	 ///< Data vector

			//------------------------------------------------------------------
			/// @brief	Constructor
			//------------------------------------------------------------------
			explicit Chunk(unsigned int level) : level_(level) {
				// Nothing to do
			}
		};

		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor for 1D caches
		//----------------------------------------------------------------------
		explicit Cache1D(const std::vector<std::vector<unsigned int> >& stack)
		: stack_(stack) {
			// Nothing to do
		}

		//----------------------------------------------------------------------
		/// @brief	Destructor
		//----------------------------------------------------------------------
		~Cache1D() {
			// Delete all the chunks
			for(unsigned int c = 0; c < chunks_.size(); ++c) {
				delete chunks_[c];
			}
		}

		//_____ Public methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Returns if the cache is empty (and pop chunks higher than
		///			the current level)
		//----------------------------------------------------------------------
		bool empty() {
			// Get the current level
			const unsigned int level = stack_.size() - 1;

			// Pop chunks too high in the stack
			while(!chunks_.empty() && chunks_.back()->level_ > level) {
				delete chunks_.back();
				chunks_.pop_back();
			}

			return chunks_.empty();
		}

		//----------------------------------------------------------------------
		/// @brief	Accesses the data
		//----------------------------------------------------------------------
		const T* data() {
			// Early return if the stack is empty
			if(empty()) {
				return 0;
			}

			// Get the current chunk
			Chunk* chunk = chunks_.back();

			// Get the current level
			unsigned int level = stack_.size() - 1;

			// Update the cache if needed
			if(chunk->level_ != level) {
				// Get the size of the new chunk
				const unsigned int size = stack_.back().size();

				// Create a new chunk
				chunks_.push_back(new Chunk(level));

				// Get a reference on it
				Chunk* previous = chunks_.back();
				std::swap(chunk, previous);

				// Resize it
				chunk->data_.resize(size);

				// Index the data of the new chunk in the previous one
				for(unsigned int i = 0; i < size; ++i) {
					register unsigned int index = i;

					for(unsigned int l = level; l > previous->level_; --l) {
						index = stack_[l][index];
					}

					chunk->data_[i] = previous->data_[index];
				}
			}

			return &chunk->data_[0];
		}

		//----------------------------------------------------------------------
		/// @brief	Swaps the content of the cache with a vector
		///
		/// If the cache is empty, swaps the data with a new chunk. If the cache
		/// is not empty, swaps the data with the last chunk. The data must
		/// either have the same size as the last chunk or be empty.
		//----------------------------------------------------------------------
		void swap(std::vector<T>& data) {
			if(empty()) {
				// Nothing to do if data is also empty
				if(data.empty()) {
					return;
				}

				chunks_.push_back(new Chunk(stack_.size() - 1));

				// Swap its data with the ones provided
				chunks_.back()->data_.swap(data);
			}
			else {
				// Make sure that the last chunk is at the current level
				this->data();

				if(data.empty()) {
					chunks_.back()->data_.swap(data);
					delete chunks_.back();
					chunks_.pop_back();
				}
				else {
					assert(data.size() == chunks_.back()->data_.size());
					chunks_.back()->data_.swap(data);
				}
			}
		}

		//----------------------------------------------------------------------
		/// @brief	Clears the cache completely (delete all the chunks)
		//----------------------------------------------------------------------
		void clear() {
			// Delete all the chunks
			for(unsigned int c = 0; c < chunks_.size(); ++c) {
				delete chunks_[c];
			}

			chunks_.clear();
		}

		//_____ Attributes _______
	private:
		const std::vector<std::vector<unsigned int> >& stack_;
		std::vector<Chunk*> chunks_;
	};

	//--------------------------------------------------------------------------
	/// @brief	A 2D cache (in reality a stack of them)
	///
	/// Caches are stored as stacks of chunks, so that they can be updated to
	/// follow the sample or feature stack(s) (the user can only see the most
	/// recent version of the cache, so that the swap cache methods simply swaps
	/// vectors).
	//--------------------------------------------------------------------------
	template <typename T>
	class Cache2D {
		//_____ Internal types _______
	private:
		//----------------------------------------------------------------------
		/// @brief	A 2D cache chunk
		//----------------------------------------------------------------------
		struct Chunk {
			const unsigned int	level1_;	///< Level in the first stack
			const unsigned int	level2_;	///< Level in the first stack
			bool				trans_;		///< Whether the chunk is transposed
			unsigned int		stride_;	///< The stride (leading dimension)
			std::vector<T>		data_;	 	///< Data vector

			//------------------------------------------------------------------
			/// @brief	Constructor
			//------------------------------------------------------------------
			Chunk(unsigned int level1,
			      unsigned int level2,
			      bool trans,
			      unsigned int stride)
			: level1_(level1), level2_(level2), trans_(trans), stride_(stride) {
				// Nothing to do
			}
		};

		//_____ Construction / Destruction and Copy _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Constructor for 2D caches
		//----------------------------------------------------------------------
		Cache2D(const std::vector<std::vector<unsigned int> >& stack1,
		        const std::vector<std::vector<unsigned int> >& stack2)
		: stack1_(stack1), stack2_(stack2) {
			// Nothing to do
		}

		//----------------------------------------------------------------------
		/// @brief	Destructor
		//----------------------------------------------------------------------
		~Cache2D() {
			// Delete all the chunks
			for(unsigned int c = 0; c < chunks_.size(); ++c) {
				delete chunks_[c];
			}
		}

		//_____ Public methods _______
	public:
		//----------------------------------------------------------------------
		/// @brief	Returns if the cache is empty (and pop chunks higher than
		///			the current level)
		//----------------------------------------------------------------------
		bool empty() {
			// Get the current levels
			const unsigned int level1 = stack1_.size() - 1;
			const unsigned int level2 = stack2_.size() - 1;

			// Pop chunks too high in the stack
			while(!chunks_.empty() && (chunks_.back()->level1_ > level1 ||
									   chunks_.back()->level2_ > level2)) {
				delete chunks_.back();
				chunks_.pop_back();
			}

			return chunks_.empty();
		}

		//----------------------------------------------------------------------
		/// @brief	Accesses the data
		//----------------------------------------------------------------------
		const T* data() {
			// Early return if the stack is empty
			if(empty()) {
				return 0;
			}

			// Get the current chunk
			Chunk* chunk = chunks_.back();

			// Update the cache if needed
			if(chunk->level1_ != stack1_.size() - 1 ||
			   chunk->level2_ != stack2_.size() - 1) {
				chunk = update(stack1_.size() - 1, stack2_.size() - 1);
			}

			// Transpose it back if it already transposed
			if(chunk->trans_) {
				unsigned int stride = stack2_.back().size();
				transpose(&chunk->data_[0], stride, chunk->stride_);
				chunk->trans_ = false;
				chunk->stride_ = stride;
			}

			return &chunk->data_[0];
		}

		//----------------------------------------------------------------------
		/// @brief	Accesses the data transposed
		//----------------------------------------------------------------------
		const T* dataT() {
			// Early return if the stack is empty
			if(empty()) {
				return 0;
			}

			// Get the current chunk
			Chunk* chunk = chunks_.back();

			// Update the cache if needed
			if(chunk->level1_ != stack1_.size() - 1 ||
			   chunk->level2_ != stack2_.size() - 1) {
				chunk = update(stack1_.size() - 1, stack2_.size() - 1);
			}

			// Transpose it if it is not already
			if(!chunk->trans_) {
				unsigned int stride = stack1_.back().size();
				transpose(&chunk->data_[0], stride, chunk->stride_);
				chunk->trans_ = true;
				chunk->stride_ = stride;
			}

			return &chunk->data_[0];
		}

		//----------------------------------------------------------------------
		/// @brief	Swaps the content of the cache with a vector
		///
		/// If the cache is empty, swaps the data with a new chunk. If the cache
		/// is not empty, swaps the data with the last chunk. The data must
		/// either have the same size as the last chunk or be empty.
		//----------------------------------------------------------------------
		void swap(std::vector<T>& data) {
			if(empty()) {
				// Nothing to do if data is also empty
				if(data.empty()) {
					return;
				}

				chunks_.push_back(new Chunk(stack1_.size() - 1,
											stack2_.size() - 1,
											false, stack2_.back().size()));

				// Swap its data with the ones provided
				chunks_.back()->data_.swap(data);
			}
			else {
				// Make sure that the last chunk is at the current level
				this->data();

				if(data.empty()) {
					chunks_.back()->data_.swap(data);
					delete chunks_.back();
					chunks_.pop_back();
				}
				else {
					assert(data.size() == chunks_.back()->data_.size());
					chunks_.back()->data_.swap(data);
				}
			}
		}

		//----------------------------------------------------------------------
		/// @brief	Swaps the content of the cache with a transposed vector
		//----------------------------------------------------------------------
		void swapT(std::vector<T>& data) {
			if(empty()) {
				// Nothing to do if data is also empty
				if(data.empty()) {
					return;
				}

				chunks_.push_back(new Chunk(stack1_.size() - 1,
											stack2_.size() - 1,
											true, stack2_.back().size()));

				// Swap its data with the ones provided
				chunks_.back()->data_.swap(data);
			}
			else {
				// Make sure that the last chunk is at the current level
				this->dataT();

				if(data.empty()) {
					chunks_.back()->data_.swap(data);
					delete chunks_.back();
					chunks_.pop_back();
				}
				else {
					assert(data.size() == chunks_.back()->data_.size());
					chunks_.back()->data_.swap(data);
				}
			}
		}

		//----------------------------------------------------------------------
		/// @brief	Clears the cache completely (deletes all the chunks)
		//----------------------------------------------------------------------
		void clear() {
			// Delete all the chunks
			for(unsigned int c = 0; c < chunks_.size(); ++c) {
				delete chunks_[c];
			}

			chunks_.clear();
		}

		//_____ Inner methods _______
	private:
		//----------------------------------------------------------------------
		/// @brief	Updates a 2D cache (that is, creates a new chunk indexed in
		///			the previous one)
		///
		/// @param	level1	The level of the new chunk in the first stack
		/// @param	level2	The level of the new chunk in the second stack
		/// @return			A pointer on the new chunk
		//----------------------------------------------------------------------
		Chunk* update(unsigned int level1,
					  unsigned int level2) {
			// Get a pointer on the previous chunk
			Chunk* previous = chunks_.back();

			// Get the sizes of the new chunk
			const unsigned int size1 = stack1_.back().size();
			const unsigned int size2 = stack2_.back().size();

			// Create a new chunk
			chunks_.push_back(new Chunk(level1, level2, previous->trans_,
										previous->trans_ ? size1 : size2));

			// Get a pointer on it
			Chunk* chunk = chunks_.back();

			// Resize it
			chunk->data_.resize(size1 * size2);

			// Index the data of the new chunk in the previous one
			if(!chunk->trans_) {
				std::vector<unsigned int> indices(size2);

				for(unsigned int i = 0; i < size2; ++i) {
					register unsigned int index = i;

					for(unsigned int l = level2; l > previous->level2_; --l) {
						index = stack2_[l][index];
					}

					indices[i] = index;
				}

				for(unsigned int i = 0; i < size1; ++i) {
					register unsigned int index = i;

					for(unsigned int l = level1; l > previous->level1_; --l) {
						index = stack1_[l][index];
					}

					for(unsigned int j = 0; j < size2; ++j) {
						chunk->data_[i * chunk->stride_ + j] =
							previous->data_[index * previous->stride_ +
											indices[j]];
					}
				}
			}
			else {
				std::vector<unsigned int> indices(size1);

				for(unsigned int i = 0; i < size1; ++i) {
					register unsigned int index = i;

					for(unsigned int l = level1; l > previous->level1_; --l) {
						index = stack1_[l][index];
					}

					indices[i] = index;
				}

				for(unsigned int i = 0; i < size2; ++i) {
					register unsigned int index = i;

					for(unsigned int l = level2; l > previous->level2_; --l) {
						index = stack2_[l][index];
					}

					for(unsigned int j = 0; j < size1; ++j) {
						chunk->data_[i * chunk->stride_ + j] =
							previous->data_[index * previous->stride_ +
											indices[j]];
					}
				}
			}

			return chunk;
		}

		//_____ Attributes _______
	private:
		const std::vector<std::vector<unsigned int> >& stack1_;
		const std::vector<std::vector<unsigned int> >& stack2_;
		std::vector<Chunk*> chunks_;
	};
} // namespace ML

#endif // ML_INPUTSET_CACHE_H

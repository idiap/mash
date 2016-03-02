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


/// \file	transpose.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 25, 2011

#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include <algorithm>
#include <vector>

/// @brief	Transposes in place a matrix defined as a one dimensional array.
///			Fast as it uses a gcd decomposition of the matrix and uses
///			little additional memory compared to the size of the matrix
///			(typically a few percents, at worst 1/32th of the original size,
///			plus O(#rows + #cols))
/// @param	data	The matrix
/// @param	rows	The number of rows of the matrix
/// @param	cols	The number of columns of the matrix
template <typename T>
void transpose(T* data,
			   const unsigned int rows,
			   const unsigned int cols) {
	// If one of the dimension is one there is nothing to do
	if (rows < 2 || cols < 2) {
		return;
	}

	// If the matrix is square it can be transposed by simply swapping
	// across the diagonal
	if (rows == cols) {
		for (unsigned int i = 0; i < rows; ++i) {
			for (unsigned int j = i + 1; j < rows; ++j) {
				std::swap(data[i * rows + j], data[i + j * rows]);
			}
		}

		return;
	}

	// Search for the decomposition with the smallest memory usage in the
	// range [max(rows - radius, 0), rows) x [max(cols - radius, 0), cols)
	const unsigned int radius = 32; // Make sure that gcd >= 32
	unsigned int memory = rows * cols + 1; // Worst-case

	// The size of the selected submatrix
	unsigned int md = rows;
	unsigned int nd = cols;
	unsigned int d = 0;

	// Make the subtraction only if rows, cols > radius
	const unsigned int mLow = (rows > radius) ? (rows - radius) : 1;
	const unsigned int nLow = (cols > radius) ? (cols - radius) : 1;

	for (unsigned int i = mLow; i < rows; ++i) {
		for (unsigned int j = nLow; j < cols; ++j) {
			// Compute the gcd of i, j
			unsigned int k = i, l = j;

			do {
				unsigned int r = k % l;
				k = l;
				l = r;
			}
			while (l);

			// Compute the memory used by the decomposition
			unsigned int mem = (i * j) / k +
							   i * (cols - j) +
							   (rows - i) * cols;

			// Store the decomposition if it uses less memory
			if (mem < memory) {
				md = i;
				nd = j;
				d = k;
				memory = mem;
			}
		}
	}

	// Divide the selected submatrix into d * d blocks
	const unsigned int m = md / d;
	const unsigned int n = nd / d;
	const unsigned int mn = m * n;
	const unsigned int dmn = d * mn;

	// Need to save the two other submatrices from the decomposition
	std::vector<T> sub1, sub2;

	if (nd != cols) {
		sub1.resize(md * (cols - nd));

		for (unsigned int i = 0; i < md; ++i) {
			std::copy(data + i * cols + nd,
					  data + i * cols + cols,
					  sub1.begin() + i * (cols - nd));
		}
	}

	if (md != rows) {
		sub2.assign(data + md * cols, data + rows * cols);
	}

	// Temporary buffer
	std::vector<T> buffer(dmn);

	// Treat the selected submatrix as a (d x m) x (d' x n) matrix

	// First, transpose d x (m x d') x n to d x (d' x m) x n, using the
	// buffer matrix. This consists of d transposes of contiguous m x d'
	// matrices
	if (m > 1) {
		for (unsigned int i = 0; i < d; ++i) {
			if (n > 1) {
				for (unsigned int j = 0; j < m; ++j) {
					for (unsigned int k = 0; k < d; ++k) {
						std::copy(data + i * dmn +
										 (i * m + j) * (cols - nd) +
										 (j * d + k) * n,
								  data + i * dmn +
										 (i * m + j) * (cols - nd) +
										 (j * d + k) * n + n,
								  buffer.begin() + (j + k * m) * n);
					}
				}
			}
			else { // std::copy is very slow to copy one element
				for (unsigned int j = 0; j < m; ++j) {
					for (unsigned int k = 0; k < d; ++k) {
						buffer[j + k * m] = data[i * dmn +
												 (i * m + j) * (cols - nd) +
												 j * d + k];
					}
				}
			}

			std::copy(buffer.begin(), buffer.end(), data + i * dmn);
		}
	}
	else if (nd != cols) {
		for (unsigned int i = 0; i < d; ++i) {
			std::copy(data + i * dmn + i * (cols - nd),
					  data + i * dmn + i * (cols - nd) + dmn,
					  data + i * dmn);
		}
	}

	// Now, transpose (d x d') x (m x n) to (d' x d) x (m x n), which
	// is a square in-place transpose of m * n elements
	if (mn > 1) {
		for (unsigned int i = 0; i < d; ++i) {
			for (unsigned int j = i + 1; j < d; ++j) {
				std::swap_ranges(data + (i * d + j) * mn,
								 data + (i * d + j) * mn + mn,
								 data + (i + j * d) * mn);
			}
		}
	}
	else {
		for (unsigned int i = 0; i < d; ++i) {
			for (unsigned int j = i + 1; j < d; ++j) {
				std::swap(data[i * d + j], data[i + j * d]);
			}
		}
	}

	// Finally, transpose d' x ((d x m) x n) to d' x (n x (d x m)),
	// using the buffer matrix. This consists of d' transposes
	// of contiguous (d * m) x n matrices
	if (n > 1) {
		for (unsigned int i = d; i; --i) {
			for (unsigned int j = 0; j < md; ++j) {
				for (unsigned int k = 0; k < n; ++k) {
					buffer[k * md + j] = data[(i - 1) * dmn + j * n + k];
				}
			}

			for (unsigned int j = n; j; --j) {
				std::copy(buffer.begin() + (j - 1) * md,
						  buffer.begin() + j * md,
						  data + (i - 1) * (dmn + n * (rows - md)) +
								 (j - 1) * rows);
			}
		}
	}
	else if (md != rows) {
		for (unsigned int i = d; i; --i) {
			std::copy_backward(data + (i - 1) * md,
							   data + i * md,
							   data + (i - 1) * rows + md);
		}
	}

	// Write the buffers transposed
	if (nd != cols) {
		for (unsigned int i = 0; i < md; ++i) {
			for (unsigned int j = nd; j < cols; ++j) {
				data[j * rows + i] = sub1[i * (cols - nd) + j - nd];
			}
		}
	}

	if (md != rows) {
		for (unsigned int i = md; i < rows; ++i) {
			for (unsigned int j = 0; j < cols; ++j) {
				data[j * rows + i] = sub2[(i - md) * cols + j];
			}
		}
	}
}

#endif // TRANSPOSE_H

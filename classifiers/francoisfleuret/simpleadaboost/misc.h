/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Francois Fleuret (francois.fleuret@idiap.ch)
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


/** Author: Francois Fleuret (francois.fleuret@idiap.ch)
*/

#ifndef MISC_H
#define MISC_H

#include <string.h>
#include <stdlib.h>

#ifndef NDEBUG
#define ASSERT(x) if(!(x)) { \
  std::cerr << "ASSERT FAILED IN " << __FILE__ << ":" << __LINE__ << endl; \
  abort(); \
}
#else
#define ASSERT(x)
#endif

template<class T>
inline void exchange(T &a, T &b) {
  T c = a;
  a = b;
  b = c;
}

template<class T>
T **allocate_array(int a, int b) {
  T *tmp = new T[a * b];
  T **array = new T *[a];
  for(int k = 0; k < a; k++) {
    array[k] = tmp;
    tmp += b;
  }
  return array;
}

template<class T>
T **allocate_clear_array(int a, int b) {
  T *tmp = new T[a * b];
  memset(tmp, 0, sizeof(T) * a * b);
  T **array = new T *[a];
  for(int k = 0; k < a; k++) {
    array[k] = tmp;
    tmp += b;
  }
  return array;
}

template<class T>
void deallocate_array(T **array) {
  delete[] array[0];
  delete[] array;
}

#endif

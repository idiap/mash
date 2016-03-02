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

    This heuristic maintains an average value for every pixel of the
    region of interest an return the gap between the actual value of
    the pixel and its average value. The average value is updated at
    every prepareForCoordinates.

 */

#include <iostream>
#include <mash/heuristic.h>
#include <stdlib.h>

using namespace Mash;

class SlidingP: public Heuristic
{
public:
  SlidingP();
  virtual ~SlidingP();

public:
  virtual void init();
  virtual void terminate();
  virtual unsigned int dim();
  virtual void prepareForImage();
  virtual void finishForImage();
  virtual void prepareForCoordinates();
  virtual void finishForCoordinates();
  virtual scalar_t computeFeature(unsigned int feature_index);

protected:
  scalar_t *_sum;
  int *_dx, *_dy;
  int _width, _height;
  static const float _alpha = 0.99;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
  return new SlidingP();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

SlidingP::SlidingP() { }


SlidingP::~SlidingP() { }


/************************* IMPLEMENTATION OF Heuristic ************************/

void SlidingP::init() {
  _width = roi_extent * 2 + 1;
  _height = _width;
  _sum = new scalar_t[_width * _height];
  _dx = new int[_width * _height];
  _dy = new int[_width * _height];
  int n = 0;
  for(int y = 0; y < _height; y++) {
    for(int x = 0; x < _width; x++) {
      _dx[n] = x - roi_extent;
      _dy[n] = y - roi_extent;
      n++;
    }
  }
}

void SlidingP::terminate() {
  delete[] _sum;
  delete[] _dx;
  delete[] _dy;
}

unsigned int SlidingP::dim() {
  return _width * _height;
}

void SlidingP::prepareForImage() { }


void SlidingP::finishForImage() { }

void SlidingP::prepareForCoordinates() { }

void SlidingP::finishForCoordinates() {
  byte_t **pixels = image->grayLines();
  for(int f = 0; f < _width * _height; f++) {
    int x = coordinates.x + _dx[f];
    int y = coordinates.y + _dy[f];
    if(x < 0 || x >= _width || y < 0 || y >= _height) {
      _sum[f] = (1 - _alpha) * _sum[f] + _alpha * float(pixels[x][y]);
    }
  }
}

scalar_t SlidingP::computeFeature(unsigned int feature_index) {
  byte_t **pixels = image->grayLines();
  int x = coordinates.x + _dx[feature_index];
  int y = coordinates.y + _dy[feature_index];

  if(x < 0 || x >= _width ||
     y < 0 || y >= _height) return 0.5;

  return float(pixels[x][y]) - _sum[feature_index];
}

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

    This heuristic computes the average level of gray over 1000 boxes
    picked at random when dim() is called (i.e. the 1000 boxes remain
    the same for all images and sub-windows).

 */

#include <mash/heuristic.h>
#include <stdlib.h>

using namespace Mash;

struct Rectangle {
  int xmin, ymin, xmax, ymax;
};

class BoxedAverages: public Heuristic {
public:
    BoxedAverages();
    virtual ~BoxedAverages();

public:
    virtual void init();
    virtual unsigned int dim();
    virtual void prepareForImage();
    virtual void finishForImage();
    virtual void prepareForCoordinates();
    virtual void finishForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);

protected:
  static const int nb_features = 1000;
  int *_integral_image;
  int _image_width, _image_height;
  Rectangle *_rectangles;
};


extern "C" Heuristic* new_heuristic() {
    return new BoxedAverages();
}

BoxedAverages::BoxedAverages() {
  _rectangles = new Rectangle[nb_features];
}


BoxedAverages::~BoxedAverages() {
  delete[] _rectangles;
}

void BoxedAverages::init() {
  for(int n = 0; n < nb_features; n++) {
    do {
      _rectangles[n].xmin = - roi_extent +
        int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].ymin = - roi_extent +
        int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].xmax = - roi_extent +
        int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].ymax = - roi_extent +
        int(drand48() * (2 * roi_extent + 1));
    } while(_rectangles[n].xmax <= _rectangles[n].xmin ||
            _rectangles[n].ymax <= _rectangles[n].ymin);
  }
}

unsigned int BoxedAverages::dim() {
  return nb_features;
}

void BoxedAverages::prepareForImage() {
  byte_t **pixels = image->grayLines();

  _image_width = image->width();

  _image_height = image->height();
  _integral_image = new int [_image_width * _image_height];

  for(int y = 0; y < _image_height; y++) {
    for(int x = 0; x < _image_width; x++) {
      if(y == 0 || x == 0) {
        _integral_image[x + _image_width * y] = 0;
      } else {
        _integral_image[x + _image_width * y] =
          pixels[x][y]
          + _integral_image[(x - 1) + _image_width * y]
          + _integral_image[x + _image_width * (y - 1)]
          - _integral_image[(x - 1) + _image_width * (y - 1)];
      }
    }
  }
}


void BoxedAverages::finishForImage() {
  delete[] _integral_image;
}

void BoxedAverages::prepareForCoordinates() { }

void BoxedAverages::finishForCoordinates() { }

scalar_t BoxedAverages::computeFeature(unsigned int feature_index) {
  int xmin = coordinates.x + _rectangles[feature_index].xmin;
  int ymin = coordinates.y + _rectangles[feature_index].ymin;
  int xmax = coordinates.x + _rectangles[feature_index].xmax;
  int ymax = coordinates.y + _rectangles[feature_index].ymax;

  if(xmin < 0) xmin = 0;
  if(xmax >= _image_width) xmax = _image_width - 1;
  if(ymin < 0) ymin = 0;
  if(ymax >= _image_height) ymax = _image_height - 1;

  if(xmax > xmin && ymax > ymin) {
    return
      scalar_t(+ _integral_image[xmin + _image_width * ymin]
               + _integral_image[xmax + _image_width * ymax]
               - _integral_image[xmax + _image_width * ymin]
               - _integral_image[xmin + _image_width * ymax]);
  } else {
    return 0.0;
  }
}

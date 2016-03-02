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

  This heuristic extracts very crude edges quantified in 8
  orientations at every pixel.

  From that, it computes 1000 features (this is an arbitrary
  parameter), each corresponding to the number of edges of certain
  orientation in a certain rectangular sub-window of the area of
  interest.

*/

#include <iostream>
#include <mash/heuristic.h>
#include <stdlib.h>

using namespace Mash;

#define PIXEL_DELTA(a, b) ((a) >= (b) ? (a) - (b) : (b) - (a))

bool edge(           byte_t v0, byte_t v1,
          byte_t v2, byte_t v3, byte_t v4, byte_t v5,
                     byte_t v6, byte_t v7) {
  byte_t g = PIXEL_DELTA(v3, v4);

  return
    g > PIXEL_DELTA(v0, v3) && g > PIXEL_DELTA(v1, v4) &&
    g > PIXEL_DELTA(v2, v3) && g > PIXEL_DELTA(v4, v5) &&
    g > PIXEL_DELTA(v3, v6) && g > PIXEL_DELTA(v4, v7);
}

struct RectangleWithOrientation {
  int e, xmin, ymin, xmax, ymax;
};

//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class Zk: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    Zk();
    virtual ~Zk();

    //_____ Implementation of Heuristic __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Called once at the creation of the heuristic
    ///
    /// Pre-computes all the data that will never change during the life-time of
    /// the heuristic
    ///
    /// When this method is called, the 'roi_extent' attribute is initialized
    ///
    /// @remark The implementation of this method is optional
    //--------------------------------------------------------------------------
    virtual void init();

    //--------------------------------------------------------------------------
    /// @brief  Called once when the heuristic is destroyed
    ///
    /// Frees the memory allocated by the init() method
    ///
    /// @remark This method must be implemented if init() is used and allocated
    ///         some memory
    //--------------------------------------------------------------------------
    // virtual void terminate();

    //--------------------------------------------------------------------------
    // Returns the number of features this heuristic computes
    //
    // When this method is called, the 'roi_extent' attribute is initialized
    //--------------------------------------------------------------------------
    virtual unsigned int dim();

    //--------------------------------------------------------------------------
    // Called once per image, before any computation
    //
    // Pre-computes from a full image the data the heuristic will need to compute
    // features at any coordinates in the image
    //
    // When this method is called, the following attributes are initialized:
    //     - roi_extent
    //     - image
    //--------------------------------------------------------------------------
    virtual void prepareForImage();

    //--------------------------------------------------------------------------
    // Called once per image, after any computation
    //
    // Frees the memory allocated by the prepareForImage() method
    //--------------------------------------------------------------------------
    virtual void finishForImage();

    //--------------------------------------------------------------------------
    // Called once per coordinates, before any computation
    //
    // Pre-computes the data the heuristic will need to compute features at the
    // given coordinates
    //
    // When this method is called, the following attributes are initialized:
    //     - roi_extent
    //     - image
    //     - coordinates
    //--------------------------------------------------------------------------
    virtual void prepareForCoordinates();

    //--------------------------------------------------------------------------
    // Called once per coordinates, after any computation
    //
    // Frees the memory allocated by the prepareForCoordinates() method
    //--------------------------------------------------------------------------
    virtual void finishForCoordinates();

    //--------------------------------------------------------------------------
    // Computes the specified feature
    //
    // When this method is called, the following attributes are initialized:
    //     - roi_extent
    //     - image
    //     - coordinates
    //--------------------------------------------------------------------------
    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
  static const int nb_edge_types = 8;
  static const int nb_features = 1000;
  int *_edge_count;
  int _image_width, _image_height;
  RectangleWithOrientation *_rectangles;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new Zk();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Zk::Zk()
{
  _rectangles = new RectangleWithOrientation[nb_features];
}


Zk::~Zk()
{
  delete[] _rectangles;
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void Zk::init()
{
  for(int n = 0; n < nb_features; n++) {
    _rectangles[n].e = int(drand48() * nb_edge_types);
    do {
      _rectangles[n].xmin = - roi_extent + int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].ymin = - roi_extent + int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].xmax = - roi_extent + int(drand48() * (2 * roi_extent + 1));
      _rectangles[n].ymax = - roi_extent + int(drand48() * (2 * roi_extent + 1));
    } while(_rectangles[n].xmax <= _rectangles[n].xmin || _rectangles[n].ymax <= _rectangles[n].ymin);
  }
}

unsigned int Zk::dim()
{
  return nb_features;
}


void Zk::prepareForImage()
{
  byte_t **pixels = image->grayLines();

  _image_width = image->width() + 1;
  _image_height = image->height() + 1;
  _edge_count = new int [_image_width * _image_height * nb_edge_types];

  int d00, d01, d02, d03, d04, d05, d06, d07;
  int d08, d09, d10, d11, d12, d13, d14, d15;

  for(int y = 0; y < _image_height; y++) {
    for(int x = 0; x < _image_width; x++) {
      if(y == 0 || x == 0) {
        for(int e = 0; e < nb_edge_types; e++)
          _edge_count[e + nb_edge_types * (x + _image_width * y)] = 0;
      } else {
        for(int e = 0; e < nb_edge_types; e++)
          _edge_count[e + nb_edge_types * (x + _image_width * y)] =
            + _edge_count[e + nb_edge_types * ((x - 1) + _image_width * y)]
            + _edge_count[e + nb_edge_types * (x + _image_width * (y - 1))]
            - _edge_count[e + nb_edge_types * ((x - 1) + _image_width * (y - 1))];
      }

      if(x > 1 && x < _image_width - 3 && y > 1 && y < _image_height - 3) {
        d00 = pixels[(x - 1)][(y - 1)];
        d01 = pixels[(x + 0)][(y - 1)];
        d02 = pixels[(x + 1)][(y - 1)];
        d03 = pixels[(x + 2)][(y - 1)];

        d04 = pixels[(x - 1)][(y + 0)];
        d05 = pixels[(x + 0)][(y + 0)];
        d06 = pixels[(x + 1)][(y + 0)];
        d07 = pixels[(x + 2)][(y + 0)];

        d08 = pixels[(x - 1)][(y + 1)];
        d09 = pixels[(x + 0)][(y + 1)];
        d10 = pixels[(x + 1)][(y + 1)];
        d11 = pixels[(x + 2)][(y + 1)];

        d12 = pixels[(x - 1)][(y + 2)];
        d13 = pixels[(x + 0)][(y + 2)];
        d14 = pixels[(x + 1)][(y + 2)];
        d15 = pixels[(x + 2)][(y + 2)];

        /*

          XXXXXX     .XXXXX     ...XXX     .....X
          XXXXXX     ..XXXX     ...XXX     ....XX
          ......     ...XXX     ...XXX     ...XXX
          ......     ....XX     ...XXX     ..XXXX

            #0         #1         #2         #3

          ......     X.....     XXX...     XXXXX.
          ......     XX....     XXX...     XXXX..
          XXXXXX     XXX...     XXX...     XXX...
          XXXXXX     XXXX..     XXX...     XX....

            #4         #5         #6         #7

        */

        if(edge(d04, d08, d01, d05, d09, d13, d06, d10)) {
          if(d05 < d09)
            _edge_count[0 + nb_edge_types * (x + _image_width * y)]++;
          else
            _edge_count[4 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d02, d07, d00, d05, d10, d15, d08, d13)) {
          if(d05 < d10)
            _edge_count[7 + nb_edge_types * (x + _image_width * y)]++;
          else
            _edge_count[3 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d01, d02, d04, d05, d06, d07, d09, d10)) {
          if(d05 < d06)
            _edge_count[6 + nb_edge_types * (x + _image_width * y)]++;
          else
            _edge_count[2 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d01, d04, d03, d06, d09, d12, d11, d14)) {
          if(d06 < d09)
            _edge_count[1 + nb_edge_types * (x + _image_width * y)]++;
          else
            _edge_count[5 + nb_edge_types * (x + _image_width * y)]++;
        }
      }
    }
  }
}


void Zk::finishForImage()
{
  delete[] _edge_count;
}


void Zk::prepareForCoordinates()
{
  // Nothing here
}

void Zk::finishForCoordinates()
{
  // Nothing here
}


scalar_t Zk::computeFeature(unsigned int feature_index)
{
  int xmin = coordinates.x + _rectangles[feature_index].xmin;
  int ymin = coordinates.y + _rectangles[feature_index].ymin;
  int xmax = coordinates.x + _rectangles[feature_index].xmax;
  int ymax = coordinates.y + _rectangles[feature_index].ymax;

  if(xmin < 0) xmin = 0;
  if(xmax >= _image_width) xmax = _image_width - 1;
  if(ymin < 0) ymin = 0;
  if(ymax >= _image_height) ymax = _image_height - 1;

  if(xmax > xmin && ymax > ymin) {
    int e = _rectangles[feature_index].e;
    return
      scalar_t(+ _edge_count[e + nb_edge_types * (xmin + _image_width * ymin)]
               + _edge_count[e + nb_edge_types * (xmax + _image_width * ymax)]
               - _edge_count[e + nb_edge_types * (xmax + _image_width * ymin)]
               - _edge_count[e + nb_edge_types * (xmin + _image_width * ymax)]);
  } else {
    return 0.0;
  }
}

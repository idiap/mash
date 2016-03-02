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

  This heuristic uses the same edge-detectors as Zk, and computes one
  Chamfer-distance map for each orientation. Hence, there are 8
  features per pixel of the area of interest, each telling the
  distance to the closer edge of that orientation in the image.

  The distance is limited to 10 pixels for computational reasons.

*/

#include <mash/heuristic.h>
#include <stdlib.h>
#include <iostream>

using namespace Mash;

using namespace std;

#define MIN(a, b) ((a) >= (b) ? (b) : (a))

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

//////////////////////////////////////////////////////////////////////

class ChamferZk: public Heuristic
{
public:
    ChamferZk();
    virtual ~ChamferZk();

public:
    virtual unsigned int dim();
    virtual void prepareForImage();
    virtual void finishForImage();
    virtual void prepareForCoordinates();
    virtual void finishForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);

protected:
  static const int nb_edge_types = 8;
  static const int chamfer_thickness = 4;

  unsigned int *_chamfer_maps;
  int _image_width, _image_height;
  void compute_edge_maps(byte_t **pixels, unsigned int *edge_maps);
  void compute_chamfer_maps(unsigned int *edge_maps, unsigned int *chamfer_maps);
};

extern "C" Heuristic* new_heuristic()
{
    return new ChamferZk();
}

//////////////////////////////////////////////////////////////////////

ChamferZk::ChamferZk() { }


ChamferZk::~ChamferZk() { }

unsigned int ChamferZk::dim() {
  unsigned int roi_size = roi_extent * 2 + 1;
  return nb_edge_types * roi_size * roi_size;
}

void ChamferZk::compute_edge_maps(byte_t **pixels, unsigned int *edge_maps) {
  int d00, d01, d02, d03, d04, d05, d06, d07;
  int d08, d09, d10, d11, d12, d13, d14, d15;

  for(int y = 0; y < _image_height; y++) {
    for(int x = 0; x < _image_width; x++) {
      for(int e = 0; e < nb_edge_types; e++)
        edge_maps[e + nb_edge_types * (x + _image_width * y)] = 0;

      if(x > 1 && x < _image_width - 2 && y > 1 && y < _image_height - 2) {
        d00 = pixels[(y - 1)][(x - 1)];
        d01 = pixels[(y - 1)][(x + 0)];
        d02 = pixels[(y - 1)][(x + 1)];
        d03 = pixels[(y - 1)][(x + 2)];

        d04 = pixels[(y + 0)][(x - 1)];
        d05 = pixels[(y + 0)][(x + 0)];
        d06 = pixels[(y + 0)][(x + 1)];
        d07 = pixels[(y + 0)][(x + 2)];

        d08 = pixels[(y + 1)][(x - 1)];
        d09 = pixels[(y + 1)][(x + 0)];
        d10 = pixels[(y + 1)][(x + 1)];
        d11 = pixels[(y + 1)][(x + 2)];

        d12 = pixels[(y + 2)][(x - 1)];
        d13 = pixels[(y + 2)][(x + 0)];
        d14 = pixels[(y + 2)][(x + 1)];
        d15 = pixels[(y + 2)][(x + 2)];

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
            edge_maps[0 + nb_edge_types * (x + _image_width * y)]++;
          else
            edge_maps[4 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d02, d07, d00, d05, d10, d15, d08, d13)) {
          if(d05 < d10)
            edge_maps[7 + nb_edge_types * (x + _image_width * y)]++;
          else
            edge_maps[3 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d01, d02, d04, d05, d06, d07, d09, d10)) {
          if(d05 < d06)
            edge_maps[6 + nb_edge_types * (x + _image_width * y)]++;
          else
            edge_maps[2 + nb_edge_types * (x + _image_width * y)]++;
        }

        if(edge(d01, d04, d03, d06, d09, d12, d11, d14)) {
          if(d06 < d09)
            edge_maps[1 + nb_edge_types * (x + _image_width * y)]++;
          else
            edge_maps[5 + nb_edge_types * (x + _image_width * y)]++;
        }
      }
    }
  }
}

void ChamferZk::compute_chamfer_maps(unsigned int *edge_maps,
                                     unsigned int *chamfer_maps) {
  for(int y = 0; y < _image_height; y++) {
    for(int x = 0; x < _image_width; x++) {
      for(int e = 0; e < nb_edge_types; e++) {
        if(edge_maps[e + nb_edge_types * (x + _image_width * y)]) {
          chamfer_maps[e + nb_edge_types * (x + _image_width * y)]
            = 0;
        } else {
          chamfer_maps[e + nb_edge_types * (x + _image_width * y)]
            = chamfer_thickness + 1;
        }
      }
    }
  }

  unsigned int d;
  int changed = 1;

  for(int t = 0; changed && t < chamfer_thickness; t++) {
    changed = 0;
    for(int y = 0; y < _image_height; y++) {
      for(int x = 0; x < _image_width; x++) {
        for(int e = 0; e < nb_edge_types; e++) {
          d = _image_width + _image_height;
          if(x > 0) {
            d = MIN(d,
                    _chamfer_maps[e + nb_edge_types * ((x-1) + _image_width * y)]);
          }
          if(x < _image_width - 1) {
            d = MIN(d,
                    _chamfer_maps[e + nb_edge_types * ((x+1) + _image_width * y)]);
          }
          if(y > 0) {
            d = MIN(d,
                    _chamfer_maps[e + nb_edge_types * (x + _image_width * (y-1))]);
          }
          if(y < _image_height - 1) {
            d = MIN(d,
                    _chamfer_maps[e + nb_edge_types * (x + _image_width * (y+1))]);
          }

          if(d + 1 < _chamfer_maps[e + nb_edge_types * (x + _image_width * y)]) {
            _chamfer_maps[e + nb_edge_types * (x + _image_width * y)] = d+1;
            changed = 1;
          }
        }
      }
    }
  }
}

void ChamferZk::prepareForImage() {
  byte_t **pixels = image->grayLines();

  _image_width = image->width();
  _image_height = image->height();
  _chamfer_maps = new unsigned int[_image_width * _image_height * nb_edge_types];

  unsigned int *edge_maps;
  edge_maps = new unsigned int[_image_width * _image_height * nb_edge_types];
  compute_edge_maps(pixels, edge_maps);
  compute_chamfer_maps(edge_maps, _chamfer_maps);
  delete[] edge_maps;
}


void ChamferZk::finishForImage() {
  delete[] _chamfer_maps;
}

void ChamferZk::prepareForCoordinates() { }

void ChamferZk::finishForCoordinates() { }

scalar_t ChamferZk::computeFeature(unsigned int feature_index)
{
  // This weird way of picking dx, dy and e so that the
  // output-as-an-image of testheuristic looks like something

  unsigned int x0 = coordinates.x - roi_extent;
  unsigned int y0 = coordinates.y - roi_extent;

  unsigned int roi_size = roi_extent * 2 + 1;

  unsigned int dx = feature_index % roi_size;
  feature_index /= roi_size;
  int e = feature_index % nb_edge_types;
  feature_index /= nb_edge_types;
  unsigned int dy = feature_index;

  return scalar_t(_chamfer_maps[e + nb_edge_types * ((x0 + dx) + _image_width * (y0 + dy))]);
}

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

    TODO: Write a description of your heuristic
*/

#include <mash/heuristic.h>

using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class Average: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    Average();
    virtual ~Average();


    //_____ Implementation of Heuristic __________
public:
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
    // TODO: Declare all the attributes you'll need here
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new Average();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Average::Average()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


Average::~Average()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int Average::dim()
{
    // TODO: Implement it
    return 1;
}


void Average::prepareForImage()
{
    // TODO: Initialization of the attributes that depend of the whole image
}


void Average::finishForImage()
{
    // TODO: Frees the memory allocated by the prepareForImage() method
}


void Average::prepareForCoordinates()
{
    // TODO: Initialization of the attributes that depend of the coordinates
}


void Average::finishForCoordinates()
{
    // TODO: Frees the memory allocated by the prepareForCoordinates() method
}


scalar_t Average::computeFeature(unsigned int feature_index)
{
  scalar_t s = 0;
  byte_t** pLines = image->grayLines();
  for(int y = coordinates.y - roi_extent; y <= coordinates.y + roi_extent; y++) {
    for(int x=coordinates.x - roi_extent; x<= coordinates.x+roi_extent; x++) {
      s += scalar_t(pLines[y][x]);
    }
  }

  // TODO: Implement it
  return s/scalar_t((2 * roi_extent + 1) * (2 * roi_extent + 1));
}

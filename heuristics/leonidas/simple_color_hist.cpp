/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Leonidas Lefakis (leonidas.lefakis@idiap.ch)
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


/** Author: Leonidas Lefakis (leonidas.lefakis@idiap.ch)

    TODO: Write a description of your heuristic
*/

#include <mash/heuristic.h>

using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class simple_color_hist: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    simple_color_hist();
    virtual ~simple_color_hist();


    //_____ Implementation of Heuristic __________
public:
   
    virtual unsigned int dim();
    virtual void prepareForCoordinates();
    virtual void finishForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    scalar_t *col_hist;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new simple_color_hist();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

simple_color_hist::simple_color_hist()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


simple_color_hist::~simple_color_hist()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Heuristic ************************/



unsigned int simple_color_hist::dim()
{
    // TODO: Implement it. As an example, here is the implementation of the
    // 'Identity' heuristic:

    // We have has many features than pixels in the region of
    // interest
    
    return 3;
}



void simple_color_hist::prepareForCoordinates()
{
      unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;

    unsigned int roi_size = roi_extent * 2 + 1;

    RGBPixel_t** pLines = image->rgbLines();
    RGBPixel_t col_pixel;
    col_hist= new scalar_t[dim()];

   	col_hist[0]=0;
	col_hist[1]=0;
	col_hist[2]=0;

	for (unsigned int i = 0; i < roi_size; ++i){
	  for (unsigned int j = 0; j < roi_size; ++j){
	    col_pixel = pLines[y0+i][x0+j];
	    col_hist[0]+=col_pixel.r/255.0;
	    col_hist[1]+=col_pixel.g/255.0;
	    col_hist[2]+=col_pixel.b/255.0;
	  }
	}

	col_hist[0]+=1.0/100000.0;
	col_hist[1]+=1.0/100000.0;
 	col_hist[2]+=1.0/100000.0;

	scalar_t col_sum=col_hist[0]+col_hist[1]+col_hist[2];
	col_hist[0]/=col_sum;
	col_hist[1]/=col_sum;
	col_hist[2]/=col_sum;
      
    
}


void simple_color_hist::finishForCoordinates()
{
         delete[] col_hist;

}

scalar_t simple_color_hist::computeFeature(unsigned int feature_index)
{
    return (scalar_t) col_hist[feature_index];
}

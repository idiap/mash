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

#include <stdlib.h>
#include <math.h>
using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class rgb_hogs: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    rgb_hogs();
    virtual ~rgb_hogs();


    //_____ Implementation of Heuristic __________
public:

    virtual unsigned int dim();
    virtual void prepareForCoordinates();
    virtual void finishForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
     scalar_t* dth_des;			
  unsigned int or_bins;			
  scalar_t cwidth;		
  unsigned int block_size;			
};	


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new rgb_hogs();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

rgb_hogs::rgb_hogs()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


rgb_hogs::~rgb_hogs()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Heuristic ************************/




unsigned int rgb_hogs::dim()
{
    unsigned int roi_size = roi_extent * 2 + 1;
  or_bins = 9;
  cwidth = 8;
  block_size=2;
  unsigned int hist1= 2+ceil(-0.5+ (scalar_t) roi_size/cwidth);
  unsigned int hist2=hist1;
  return (hist1-2-(block_size-1))*(hist2-2-(block_size-1))*or_bins*block_size*block_size;

}


void rgb_hogs::prepareForCoordinates()
{
    const unsigned int x0 = coordinates.x - roi_extent;
    const unsigned int y0 = coordinates.y - roi_extent;
    const unsigned int roi_size = roi_extent * 2 + 1;
    RGBPixel_t** pixels = image->rgbLines();
  
    const scalar_t pi = 3.1415926536;
    const scalar_t bin_size = pi/or_bins;
    const int hist1= 2+ceil(-0.5 + (scalar_t) roi_size/cwidth);
    const int hist2=hist1;
  
    scalar_t block[block_size][block_size][or_bins];
    scalar_t block_norm;
    scalar_t dx[3],dy[3],grad_or,grad_mag,temp_mag;
    scalar_t Xc,Yc,Oc;
    int x1,x2,y1,y2,bin1,bin2;
    int h_c;
    scalar_t h[hist1][hist2][or_bins];
    for (int x = 0; x<hist1; x++){
      for (int y =0 ; y<hist2; y++){
    for (int k = 0 ; k<or_bins; k++)  h[y][x][k]=0;
      }
    }
   
    dth_des=new scalar_t[dim()];

    //Calculate gradients (zero padding)

    for(unsigned int y=0; y<roi_size; y++) {
      for(unsigned int x=0; x<roi_size; x++) {
    if(x==0){
      dx[0] = pixels[y0+y][x0+x+1].r;
      dx[1] = pixels[y0+y][x0+x+1].g;
      dx[2] = pixels[y0+y][x0+x+1].b;
    }
    else{
      if (x==roi_size-1){
        dx[0] = -pixels[y0+y][x0+x-1].r;
        dx[1] = -pixels[y0+y][x0+x-1].g;
        dx[2] = -pixels[y0+y][x0+x-1].b;
      }
      else{
        dx[0] = pixels[y0+y][x0+x+1].r - pixels[y0+y][x0+x-1].r;
        dx[1] = pixels[y0+y][x0+x+1].g - pixels[y0+y][x0+x-1].g;
        dx[2] = pixels[y0+y][x0+x+1].b - pixels[y0+y][x0+x-1].b;
      }
    }
    if(y==0){
      dy[0] = -pixels[y0+y+1][x0+x].r;
      dy[1] = -pixels[y0+y+1][x0+x].g;
      dy[2] = -pixels[y0+y+1][x0+x].b;
    }
    else{
      if (y==roi_size-1){
        dy[0] = pixels[y0+y-1][x0+x].r;
        dy[1] = pixels[y0+y-1][x0+x].g;
        dy[2] = pixels[y0+y-1][x0+x].b;
      }
      else{
        dy[0] = -pixels[y0+y+1][x0+x].r + pixels[y0+y-1][x0+x].r;
        dy[1] = -pixels[y0+y+1][x0+x].g + pixels[y0+y-1][x0+x].g;
        dy[2] = -pixels[y0+y+1][x0+x].b + pixels[y0+y-1][x0+x].b;
      }
    }
grad_mag=0;
grad_or=0;
    for (unsigned int cli=0;cli<3;++cli){
       temp_mag= sqrt(dx[cli]*dx[cli] + dy[cli]*dy[cli]);
if (temp_mag>grad_mag){
 grad_mag=temp_mag;
grad_or= atan2(dy[cli],dx[cli]);
      if (grad_or<0) grad_or+=pi;
}
}

    //Trilinear interpolation
    bin1 = (int)floor(0.5 + grad_or/bin_size);
    bin2 = bin1 + 1;
    x1   = (int)floor(0.5+ x/cwidth);
    x2   = x1+1;
    y1   = (int)floor(0.5+ y/cwidth);
    y2   = y1 + 1;
    
    Xc = (x1+1-1.5)*cwidth + 0.5;
    Yc = (y1+1-1.5)*cwidth + 0.5;
    
    Oc = (bin1+1-1.5)*bin_size;
    if (bin2==or_bins){
      bin2=0;
    }
    if (bin1==0){
      bin1=or_bins-1;
    }

    h[y1][x1][bin1]= h[y1][x1][bin1]+grad_mag*(1-((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
    h[y1][x1][bin2]= h[y1][x1][bin2]+grad_mag*(1-((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
    h[y2][x1][bin1]= h[y2][x1][bin1]+grad_mag*(1-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
    h[y2][x1][bin2]= h[y2][x1][bin2]+grad_mag*(1-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
    h[y1][x2][bin1]= h[y1][x2][bin1]+grad_mag*(((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
    h[y1][x2][bin2]= h[y1][x2][bin2]+grad_mag*(((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
    h[y2][x2][bin1]= h[y2][x2][bin1]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
    h[y2][x2][bin2]= h[y2][x2][bin2]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
      }
    }
   
   

    //Block normalization
    h_c=0;
    for(unsigned int x=1; x<hist1-block_size; x++){
      for (unsigned int y=1; y<hist2-block_size; y++){
    block_norm=0;
    for (unsigned int i=0; i<block_size; i++){
      for(unsigned int j=0; j<block_size; j++){
        for(unsigned int k=0; k<or_bins; k++){
          block_norm+=h[y+i][x+j][k]*h[y+i][x+j][k];
        }
      }
    }
    block_norm=sqrt(block_norm);
    for (unsigned int i=0; i<block_size; i++){
      for(unsigned int j=0; j<block_size; j++){
        for(unsigned int k=0; k<or_bins; k++){
          if (block_norm>0){
        block[i][j][k]=h[y+i][x+j][k]/block_norm;
        if (block[i][j][k]>0.2) block[i][j][k]=0.2;
          }
        }
      }
    }
    block_norm=0;
    for (unsigned int i=0; i<block_size; i++){
      for(unsigned int j=0; j<block_size; j++){
        for(unsigned int k=0; k<or_bins; k++){
          block_norm+=block[i][j][k]*block[i][j][k];
        }
      }
    }
    block_norm=sqrt(block_norm);
    for (unsigned int i=0; i<block_size; i++){
      for(unsigned int j=0; j<block_size; j++){
        for(unsigned int k=0; k<or_bins; k++){
          if (block_norm>0) dth_des[h_c]=block[i][j][k]/block_norm;
          else dth_des[h_c]=0;
          h_c++;
        }
      }
    }
      }
    }
}


void rgb_hogs::finishForCoordinates()
{
   delete[] dth_des;
}


scalar_t rgb_hogs::computeFeature(unsigned int feature_index)
{
    return  dth_des[feature_index];
}

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
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace Mash;
using namespace std;

//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class laws: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    laws();
    virtual ~laws();


    //_____ Implementation of Heuristic __________
public:
  
    virtual void init();

    virtual unsigned int dim();
    virtual void prepareForImage();
    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:

    vector<vector<scalar_t> > LESWR_c; 
    vector<vector<vector<scalar_t> > > Energy_Images;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new laws();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

laws::laws()
{
    scalar_t L[] = { 1, 2, 1};
    scalar_t E[] = {-1, 0, 1};
    scalar_t S[] = {-1, 2, -1};
    
    vector<vector<scalar_t> > LESWR;
    LESWR.resize(3);
    for (unsigned int i=0;i<3;i++) LESWR[i].resize(3);
    
    
    LESWR[0].assign(L, L+2);
    LESWR[1].assign(E, E+2);
    LESWR[2].assign(S, S+2);
   
    
    LESWR_c.resize(9);
    for(unsigned int i=0;i<3;i++){
        for(unsigned int m=0;m<3;m++){
            LESWR_c[i*3+m].resize(9);
            for(unsigned int k=0;k<3;k++){
                for(unsigned int j=0;j<3;j++){
                    LESWR_c[i*3+m][3*k + j]=LESWR[i][k]*LESWR[m][j];
                }
            }
        }
    }
}

laws::~laws()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void laws::init()
{
}


unsigned int laws::dim()
{
    // TODO: Implement it. As an example, here is the implementation of the
    // 'Identity' heuristic:

    // We have has many features than pixels in the region of
    // interest
    unsigned int roi_size = roi_extent * 2 + 1;
    return roi_size * roi_size*9;
}


void laws::prepareForImage()
{
    byte_t** pixels = image->grayLines();
    Energy_Images.resize(9);
    for (unsigned int i=0;i<9;i++){
        Energy_Images[i].resize(image->height());
        for(unsigned int y=0; y<image->height(); y++) {
            Energy_Images[i][y].resize(image->width());
            for(unsigned int x=0; x<image->width(); x++) {
                Energy_Images[i][y][x]=0;
                for(unsigned int k=0; k<9; k++) {
                    if((y - 1 + (k/3)>=0) && (x -1 +(k % 3)>=0) && (y - 1 + (k/3)<image->height()) && (x -1 +(k % 3)<image->width()))
                        Energy_Images[i][y][x]+=LESWR_c[i][k]*pixels[y-1 + (k/3)][x -1 +(k % 3)];
                }

            }
        }
    }
    
    scalar_t tezei;
    
    for (unsigned int i=0;i<9;i++){
        for(unsigned int y=0; y<image->height(); y++) {
            for(unsigned int x=0; x<image->width(); x++) {
                tezei=0;
                for(unsigned int k=0; k<3; k++) {
                    for(unsigned int m=0; m<3; m++) {                        
                        if ((y +1 - k>=0) && (x +1 - m>=0) && (y +1 - k<image->height()) && (x +1 -m<image->width())){
                            if (Energy_Images[i][y+1-k][x +1-m]>0) tezei+=Energy_Images[i][y+1-k][x+1-m]/9;
                            else tezei-=Energy_Images[i][y+1-k][x+1-m]/9;                    
                        }
                    }                                      
                }
                Energy_Images[i][y][x]=tezei;
                if ((i>0) && (Energy_Images[0][y][x]>0)) Energy_Images[i][y][x]/=Energy_Images[0][y][x];
                 
            }
        }
    }

}



scalar_t laws::computeFeature(unsigned int feature_index)
{
    // TODO: Implement it. As an example, here is the implementation of the
    // 'Identity' heuristic: 

    // Compute the coordinates of the top-left pixel of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;

    // Compute the coordinates of the pixel corresponding to the feature, in
    // the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;

    unsigned int fe = feature_index / (roi_size*roi_size);
    unsigned int fx = feature_index % (roi_size*roi_size);
    unsigned int x = fx % roi_size;
    unsigned int y = (fx - x) / roi_size;

    return (scalar_t) Energy_Images[fe][y0 + y][x0 + x];
}

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
class lopd: public Heuristic
{
    //typedef vector<scalar_t> scalar2D_t;
    //typedef vector<scalar2D_t> scalar3D_t;
    
    //_____ Construction / Destruction __________
public:
    lopd();
    virtual ~lopd();


    //_____ Implementation of Heuristic __________
public:
    virtual void init();
    virtual unsigned int dim();
    virtual void prepareForImage();
    virtual void prepareForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);

    
    //_____ Attributes __________
protected:
    unsigned int or_bins;
    unsigned int probe_sigma;
    vector<vector<vector<scalar_t> > > probe_im;
    unsigned int delta;
    vector<vector<scalar_t> > probe_pos;
    vector<scalar_t> lopd_descr;
    scalar_t lopd_norm;

};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
    return new lopd();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

lopd::lopd() {
    // TODO: Initialization of the attributes that doesn't depend of anything
}


lopd::~lopd() {
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void lopd::init() {
    or_bins = 5;
    probe_sigma = 4;
    delta = 7;
}



unsigned int lopd::dim() {
    // TODO: Implement it. As an example, here is the implementation of the
    // 'Identity' heuristic:
    
    // We have has many features than pixels in the region of
    // interest
    return or_bins*(3*delta*(delta+1)+1);
}


void lopd::prepareForImage() {
    byte_t** pixels = image->grayLines();
    
    const scalar_t pi = 3.1415926536;
    const scalar_t bin_size = pi/or_bins;
    lopd_descr.assign( dim(), 0.0);

    scalar_t dx, dy, grad_or, grad_mag;
    scalar_t Oc;
    int  bin1, bin2;
    int h_c;
    
    scalar_t h[image->width()][image->height()][or_bins];
    

    probe_im.resize(image->width());
    for (int i = 0; i < image->width(); ++i) {
        probe_im[i].resize(image->height());
        for (int j = 0; j < image->height(); ++j)
            probe_im[i][j].resize(or_bins);
    }

  
    for (int x = 0; x<image->width(); x++){
        for (int y =0 ; y<image->height(); y++){
            for (int k = 0 ; k<or_bins; k++)  h[x][y][k]=0;
        }
    }
    
    
    //Calculate gradients (zero padding)
    for(unsigned int y=0; y<image->height(); y++) {
        for(unsigned int x=0; x<image->width(); x++) {
            if(x==0){
                dx = pixels[y][x+1];
            }
            else{
                if (x==image->width()-1){
                    dx = -pixels[y][x-1];
                }
                else{
                    dx = pixels[y][x+1] - pixels[y][x-1];
                }
            }
            if(y==0){
                dy = -pixels[y+1][x];
            }
            else{
                if (y==image->height()-1){
                    dy = pixels[y-1][x];
                }
                else{
                    dy = -pixels[y+1][x] + pixels[y-1][x];
                }
            }
            dx=dx/255;
            dy=dy/255;
            grad_mag= sqrt(dx*dx + dy*dy);
            if(grad_mag>0){
                grad_or= atan2(dy, dx);
                if (grad_or<0) grad_or+=pi;
            }
            else{
                grad_or=0;
            }
            
            //linear interpolation
            bin1 = (int)floor(0.5 + grad_or/bin_size);
            bin2 = bin1 + 1;
            
            
            Oc = (bin1+1-1.5)*bin_size;
            if (bin2==or_bins){
                bin2=0;
            }
            if (bin1==0){
                bin1=or_bins-1;
            }
            
            h[x][y][bin1]= grad_mag*(1-((grad_or-Oc)/bin_size));
            h[x][y][bin2]= grad_mag*((grad_or-Oc)/bin_size);
        }
    }
    
    

    int ikx,iky;
    scalar_t pre_exp[2*probe_sigma+1][2*probe_sigma+1];
    for(float kx=0; kx<=2*probe_sigma; kx++) {
        for(float ky=0; ky<=2*probe_sigma; ky++) {
            ikx = (int) kx;
            iky = (int) ky;
            pre_exp[ikx][iky]=exp(-(((kx-probe_sigma)*(kx-probe_sigma))+((ky-probe_sigma)*(ky-probe_sigma)))/(2*probe_sigma*probe_sigma));
        }
    }
    
    for(int y=0; y<image->height(); y++) {
        for(int x=0; x<image->width(); x++) {
            for(int binx=0; binx<or_bins; binx++) {
                probe_im[x][y][binx]=0;

                for(int kx=0; kx<=2*probe_sigma; kx++) {
                    for(int ky=0; ky<=2*probe_sigma; ky++) {
                        ikx= x+kx-probe_sigma;
                        iky= y+ky-probe_sigma;
                        if ((ikx>=0) && (ikx<image->width()) && (iky>=0) && (iky<image->height())) {
                            probe_im[x][y][binx]+=pre_exp[kx][ky]*h[ikx][iky][binx];
                     
                        }
                                                
                    }
                }
            }
        }
    }
    probe_pos.resize(3*delta*(delta+1)+1);
    for (int i = 0; i < 3*delta*(delta+1)+1; ++i) {
        probe_pos[i].resize(2);       
    }
    
   
    probe_pos[0][0]=0.0;
    probe_pos[0][1]=0.0;
       
    scalar_t samp_ang;
    unsigned int cindx=1;
    for (unsigned int k = 1; k<=delta;k++){
        for (unsigned int j = 0; j< 6*k;j++){
            samp_ang = j * 2*pi / (6*k);
            probe_pos[cindx][0] =  probe_sigma*k * sin(samp_ang);
            probe_pos[cindx][1] =  probe_sigma*k * cos(samp_ang);
            cindx++;
        }
    }
}

void lopd::prepareForCoordinates()
{
    const unsigned int x0 = coordinates.x;
    const unsigned int y0 = coordinates.y;
    const unsigned int roi_size = roi_extent * 2 + 1;
    int ix,iy;
    
    lopd_norm = 0.0;
    unsigned int dindx=0;
    for (unsigned int binx=0;binx<or_bins;binx++){
        for (unsigned int cindx=0;cindx<3*delta*(delta+1)+1;cindx++){
            ix = (int) x0+probe_pos[cindx][0];
            iy = (int) y0+probe_pos[cindx][1];
            if ((ix>=0) && (probe_pos[cindx][0]<roi_extent) && (iy>=0) && (probe_pos[cindx][1]<roi_extent)) {                
                lopd_descr[dindx] = probe_im[ix][iy][binx];
            }
            else lopd_descr[dindx]=0.0;
            lopd_norm+=lopd_descr[dindx]*lopd_descr[dindx];
            dindx++;    
        }
    }
}


scalar_t lopd::computeFeature(unsigned int feature_index)
{
    
    if (lopd_norm>0.0) return lopd_descr[feature_index]/lopd_norm;
    return lopd_descr[feature_index];
}

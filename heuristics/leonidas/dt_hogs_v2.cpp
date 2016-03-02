/** Author: YOUR_USERNAME

    TODO: Write a description of your heuristic
*/

#include <mash/heuristic.h>
#include <stdio.h>
#include <math.h>
using namespace Mash;



class dt_hogs: public Heuristic
{
   
public:
  dt_hogs();
  virtual unsigned int dim();
  virtual void prepareForCoordinates();
  virtual void finishForCoordinates();
  virtual scalar_t computeFeature(unsigned int feature_index);


protected:
  scalar_t* dth_des;
};


extern "C" Heuristic* new_heuristic()
{
    return new dt_hogs();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

dt_hogs::dt_hogs()
{
}


unsigned int dt_hogs::dim()
{
  
  unsigned int roi_size = roi_extent * 2 + 1;
  const int or_bins = 9;
  const int cwidth = 8;
  const int hist1= 2+(int)(roi_size/cwidth);
  const int hist2=hist1;
  return (hist1-2)*(hist2-2)*or_bins*4;
}


void dt_hogs::prepareForCoordinates()
{
    const unsigned int x0 = coordinates.x - roi_extent;
    const unsigned int y0 = coordinates.y - roi_extent;
    const unsigned int roi_size = roi_extent * 2 + 1;
    byte_t** pixels = image->grayLines();

    const scalar_t pi = 3.1415926536;
    const int or_bins = 9;
    const scalar_t bin_size = pi/or_bins;
    const int cwidth = 8;
    const int hist1= 2+(int)(roi_size/cwidth);
    const int hist2=hist1;

    scalar_t block[2][2][or_bins];
    scalar_t block_norm;
    scalar_t dx,dy,grad_or,grad_mag;
    scalar_t Xc,Yc,Oc;
    int  x1,x2,y1,y2,bin1,bin2,h_c;
    scalar_t h[hist1][hist2][or_bins];
     
   
    dth_des=new scalar_t[dim()];

    //Calculate gradients (zero padding)
    for(unsigned int y=0; y<roi_size; y++) {
      for(unsigned int x=0; x<roi_size; x++) {
	if(x==0){
	  dx = pixels[y0+y][x0+x+1];
	}
	else{
	  if (x==roi_size-1){
	    dx = -pixels[y0+y][x0+x-1];
	  }
	  else{
	    dx = pixels[y0+y][x0+x+1] - pixels[y0+y][x0+x-1];
	  }
	}
	if(y==0){
	  dy = pixels[y0+y+1][x0+x];
	  }
	else{
	  if (y==roi_size-1){
	    dy = -pixels[y0+y-1][x0+x];
	  }
	  else{
	    dy = pixels[y0+y+1][x0+x] - pixels[y0+y-1][x0+x];
	  }
	}
      
	grad_mag= sqrt(dx*dx + dy*dy);
	if(grad_mag>0){
	  grad_or= atan2(dy, dx);
	}
	else{
	  grad_or=0;
	}

	//Trilinear interpolation
	bin1 = (int)floor(0.5 + grad_or/bin_size);
	bin2 = bin1 + 1;
	x1   = (int)floor(0.5+x/cwidth);
	x2   = x1+1;
	y1   = (int)floor(0.5+y/cwidth);
	y2   = y1 + 1;
	
	Xc = (x1+1-1.5)*cwidth + 0.5;
	Yc = (y1+1-1.5)*cwidth + 0.5;
	Oc = (bin1+1-1.5)*bin_size;
	
	if (bin2==or_bins+1){
	  bin2=1;
	}
	if (bin1==0){
	  bin1=or_bins;
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
    for(unsigned int x=1; x<hist1-2; x++){
      for (unsigned int y=1; y<hist2-2; y++){
	block_norm=0;
	for (unsigned int i=0; i<2; i++){
	  for(unsigned int j=0; j<2; j++){
	    for(unsigned int k=0; k<or_bins; k++){
	      block_norm+=h[y+i][x+j][k]*h[y+i][x+j][k];
	    }
	  }
	}
	block_norm=sqrt(block_norm);
	for (unsigned int i=0; i<2; i++){
	  for(unsigned int j=0; j<2; j++){
	    for(unsigned int k=0; k<or_bins; k++){
	      block[i][j][k]=h[y+i][x+j][k]/block_norm;
	      if (block[i][j][k]>0.2) block[i][j][k]=0.2;
	    }
	  }
	}
	block_norm=0;
	for (unsigned int i=0; i<2; i++){
	  for(unsigned int j=0; j<2; j++){
	    for(unsigned int k=0; k<or_bins; k++){
	      block_norm+=block[i][j][k]*block[i][j][k];
	    }
	  }
	}
	block_norm=sqrt(block_norm);
	for (unsigned int i=0; i<2; i++){
	  for(unsigned int j=0; j<2; j++){
	    for(unsigned int k=0; k<or_bins; k++){
	      dth_des[h_c]=block[i][j][k]/block_norm;
	      h_c++;
	    }
	  }
	}
      }
    }
}


void dt_hogs::finishForCoordinates()
{
     delete[] dth_des;
}


scalar_t dt_hogs::computeFeature(unsigned int feature_index)
{
   
    return (scalar_t) dth_des[feature_index];
}

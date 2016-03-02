/** Author Flavio Tarsetti (Flavio.Tarsetti@idiap.ch)

    Declaration of the heuristic class                                     
    This heuristics computes an Local Binary Pattern (ILBP).                 
    It compares each pixel values to the mean pixel value.                      
                                                                           
         x x x                                                             
         x o x                                                             
         x x x                                                             
                                                                           
    The pixels 'x' are compared to the mean value of the pixel 'o' and 'x'              
    Result returned is an ILBP code value (8 bits example : 00100111).      
    if x is greater than the mean value the returned value is 1 otherwise 0.            
                                                                           
       example :                                                           
       110 160 205                                                         
       111 150 130                                                         
       90  155 85                                                          
                                                                           
       pixels are compared to the mean pixel value 133                   
       so the ILBP code will be [01100010]                  
 */

#include <mash/heuristic.h>
#include <stdlib.h>

using namespace Mash;


class ILBP: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    ILBP();
    virtual ~ILBP();


    //_____ Implementation of Heuristic __________
public:
    //--------------------------------------------------------------------------
    // Returns the number of features this heuristic computes
    //
    // When this method is called, the 'roi_extent' attribute is initialized
    //--------------------------------------------------------------------------
    virtual unsigned int dim();

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

};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new ILBP();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

ILBP::ILBP()
{
}


ILBP::~ILBP()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int ILBP::dim()
{
    unsigned int roi_size = roi_extent * 2 +1 ;
    return (roi_size-2)*(roi_size-2);
}

scalar_t ILBP::computeFeature(unsigned int feature_index)
{
    unsigned int x0 = coordinates.x - roi_extent ;
    unsigned int y0 = coordinates.y - roi_extent ;

    div_t v = div(feature_index, (roi_extent * 2 +1 -2)) ;
    unsigned int y = v.quot+1 ;
    unsigned int x = v.rem+1 ;

    byte_t ** pLines = image->grayLines() ;


    unsigned char result = 0 ;
    int countBits = 7 ;
    
    int num_pixels = 0 ;
    float mean_pixels_value = 0 ;
    for(int i=-1 ; i<2 ; i++)
    {
        for(int j=-1 ; j<2 ; j++)
        {
            num_pixels++ ;
            mean_pixels_value += pLines[y0 + (y+j)][x0 + (x+i)] ;
           
        }
    }
    mean_pixels_value/= num_pixels ;
    
    for(int i=-1 ; i<2 ; i++)
    {
        for(int j=-1 ; j<2 ; j++)
        {

            if(pLines[y0 + (y+j)][x0 + (x+i)] > mean_pixels_value)
                result |= 1 << countBits ;
           
            countBits -- ;
	
        }
    }


    return result ;
}

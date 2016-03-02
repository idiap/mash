/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    TODO: Write a description of your heuristic
*/

#include <mash/heuristic.h>
#include <memory.h>
#include <math.h>

using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class RedSplit: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    RedSplit();
    virtual ~RedSplit();


    //_____ Implementation of Heuristic __________
public:
    float red_sum(unsigned int left, unsigned int top,
                  unsigned int right, unsigned int bottom);


    //_____ Implementation of Heuristic __________
public:
    virtual void init();
    virtual void terminate();

    virtual unsigned int dim();

    virtual void prepareForImage();
    virtual void finishForImage();

    virtual void prepareForCoordinates();
    virtual void finishForCoordinates();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    scalar_t features[9];
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new RedSplit();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

RedSplit::RedSplit()
{
}


RedSplit::~RedSplit()
{
}


/*********************************** METHODS **********************************/

float RedSplit::red_sum(unsigned int left, unsigned int top,
                        unsigned int right, unsigned int bottom)
{
    unsigned int s = 0;
    unsigned int counter = 0;
    
    RGBPixel_t** pLines = image->rgbLines();
    
    for (unsigned int y = top; y < bottom; ++y)
    {
        for (unsigned int x = left; x < right; ++x)
        {
            if ((pLines[y][x].r > pLines[y][x].g + 20) && (pLines[y][x].r > pLines[y][x].b + 20))
            {
                s += pLines[y][x].r;
                ++counter;
            }
        }
    }

    if (counter > 10)
        return float(s) / counter;
    else
        return 0.0f;
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void RedSplit::init()
{
}


void RedSplit::terminate()
{
}


unsigned int RedSplit::dim()
{
    return 9;
}


void RedSplit::prepareForImage()
{
    memset(features, 0, 9 * sizeof(scalar_t));

    unsigned int INCREMENT = std::min(20, (int) floor(image->width() / 8));

    for (unsigned int i = 0; i < 3; ++i)
    {
        unsigned int SIGMA = INCREMENT * (i + 1);

        unsigned int split = image->width() / 2 - SIGMA;

        float avg_sum_left = red_sum(0, 0, split, image->height());
        float avg_sum_middle = red_sum(split, 0, split + 2 * SIGMA, image->height());
        float avg_sum_right = red_sum(image->width() - split, 0, image->width(), image->height());

        if ((avg_sum_left > avg_sum_middle) && (avg_sum_left > avg_sum_right))
           features[i * 3 + 0] = 1;
        else if ((avg_sum_left < avg_sum_middle) && (avg_sum_middle > avg_sum_right))
           features[i * 3 + 1] = 1;
        else if ((avg_sum_right > avg_sum_middle) && (avg_sum_right > avg_sum_left))
           features[i * 3 + 2] = 1;
    }
}


void RedSplit::finishForImage()
{
}


void RedSplit::prepareForCoordinates()
{
}


void RedSplit::finishForCoordinates()
{
}


scalar_t RedSplit::computeFeature(unsigned int feature_index)
{
    return features[feature_index];
}

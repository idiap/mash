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

    Heuristic handling all the "hard" work on behalf of a very simple one,
    written online in only one method
*/

#include <mash/simple_heuristic_container.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <deque>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace Mash;
using namespace std;


/****************************** SIMPLE HEURISTIC ******************************/

//------------------------------------------------------------------------------
/// The simple heuristic class, edited online
//------------------------------------------------------------------------------
class SimpleHeuristic: public ISimpleHeuristic
{
    //_____ Construction / Destruction __________
public:
    SimpleHeuristic();
    virtual ~SimpleHeuristic();

    //_____ Method to implement __________
public:
    virtual unsigned int dim();
    virtual void init();
    virtual void computeFeatures(unsigned int nb_images, Image** images, scalar_t* features);
    virtual void terminate();

    //_____ Attributes __________
public:
    ///ATTRIBUTES_BEGIN
//_____ Types __________

enum tEdge
{
    EDGE_NONE,
    EDGE_HORIZONTAL,
    EDGE_VERTICAL,
    EDGE_DIAGONAL_FROM_TOP_LEFT,
    EDGE_DIAGONAL_FROM_BOTTOM_LEFT,
};


struct tGradient
{
    unsigned int width;
    unsigned int height;
    unsigned char* magnitude;
    tEdge* direction;
};


//_____ Attributes __________

tGradient gradient;


//_____ Constants __________

static const unsigned int NB_BANDS = 10;



//_____ Functions __________

void sobel(Image* image)
{
    byte_t** pLines = image->grayLines();
    
    for (unsigned int y = 1; y < image->height() - 1; ++y)
    {
        for (unsigned int x = 1; x < image->width() - 1; ++x)
        {
            int gradient_x = (int) pLines[y-1][x-1] - (int) pLines[y-1][x+1]
                             + 2 * (int) pLines[y][x-1] - 2 * (int) pLines[y][x+1]
                             + (int) pLines[y+1][x-1] - (int) pLines[y+1][x+1];
    
            int gradient_y = (int) pLines[y-1][x-1] + 2 * (int) pLines[y-1][x] + (int) pLines[y-1][x+1]
                             - (int) pLines[y+1][x-1] - 2 * (int) pLines[y+1][x] - (int) pLines[y+1][x+1];
    
            gradient.magnitude[(y-1) * gradient.width + x - 1] = sqrt(gradient_x * gradient_x + gradient_y * gradient_y);
            
            float direction = atan2(gradient_y, gradient_x);
            
            tEdge edge = EDGE_NONE;
            if (gradient.magnitude[(y-1) * gradient.width + x - 1] > 20)
            {
                if ((direction > -M_PI / 8) && (direction <= M_PI * 8))
                    edge = EDGE_HORIZONTAL;
                else if ((direction > M_PI / 8) && (direction <= M_PI * 3 / 8))
                    edge = EDGE_DIAGONAL_FROM_BOTTOM_LEFT;
                else if ((direction > M_PI * 3 / 8) && (direction <= M_PI * 5 / 8))
                    edge = EDGE_VERTICAL;
                else if ((direction > M_PI * 5 / 8) && (direction <= M_PI * 7 / 8))
                    edge = EDGE_DIAGONAL_FROM_TOP_LEFT;
                else if (direction > M_PI * 7 / 8)
                    edge = EDGE_HORIZONTAL;
                else if ((direction < -M_PI / 8) && (direction >= -M_PI * 3 / 8))
                    edge = EDGE_DIAGONAL_FROM_TOP_LEFT;
                else if ((direction < -M_PI * 3 / 8) && (direction >= -M_PI * 5 / 8))
                    edge = EDGE_VERTICAL;
                else if ((direction < -M_PI * 5 / 8) && (direction >= -M_PI * 7 / 8))
                    edge = EDGE_DIAGONAL_FROM_BOTTOM_LEFT;
                else if (direction < -M_PI * 7 / 8)
                    edge = EDGE_HORIZONTAL;
            }
            
            gradient.direction[(y-1) * gradient.width + x - 1] = edge;
        }
    }
}


void filter_directions(tEdge edge, tEdge* directions,
                       unsigned int width, unsigned int height, unsigned char* results)
{
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            results[y * width + x] = (directions[y * width + x] == edge ? 255 : 0);
        }
    }
}
    ///ATTRIBUTES_END
};


SimpleHeuristic::SimpleHeuristic()
{
}


SimpleHeuristic::~SimpleHeuristic()
{
}


unsigned int SimpleHeuristic::dim()
{
    ///DIM_BEGIN
// Max: 10
return NB_BANDS;
    ///DIM_END
}


void SimpleHeuristic::init()
{
    ///INIT_BEGIN
gradient.width = 0;
gradient.height = 0;
gradient.magnitude = 0;
gradient.direction = 0;
    ///INIT_END
}


void SimpleHeuristic::computeFeatures(unsigned int nb_images, Image** images,
                                      scalar_t* features)
{
    ///COMPUTE_FEATURES_BEGIN
if (!gradient.magnitude)
{
    gradient.width = images[0]->width() - 2;
    gradient.height = images[0]->height() - 2;
    gradient.magnitude = new unsigned char[gradient.width * gradient.height];
    gradient.direction = new tEdge[gradient.width * gradient.height];
}


// Edge detection
sobel(images[0]);

// Only consider the edges at -45 degrees
unsigned char* buffer = new unsigned char[gradient.width * gradient.height];
filter_directions(EDGE_DIAGONAL_FROM_TOP_LEFT, gradient.direction, gradient.width, gradient.height, buffer);

// Compute the number of those edges in each band
unsigned int band_width = images[0]->width() / NB_BANDS;
for (unsigned int i = 0; i < NB_BANDS; ++i)
{
    features[i] = 0.0f;
    
    for (unsigned int y = images[0]->height() / 3; y < 2 * images[0]->height() / 3; ++y)
    {
        for (unsigned int x = i * band_width; x < (i  + 1) * band_width; ++x)
        {
            if ((x == 0) || (x > gradient.width))
                continue;

            if (buffer[y * gradient.width + (x - 1)] > 0)
                features[i] += 1.0f;
        }
    }
    
    // logger << features[i] << std::endl;
}


// DEBUG
unsigned int nb_edges = 0;
for (unsigned int i = 0; i < NB_BANDS; ++i)
    nb_edges += features[i];

unsigned char dbg_buffer[(10 * 10) * NB_BANDS];
for (unsigned int i = 0; i < NB_BANDS; ++i)
{
    float percentage = float(features[i]) / nb_edges;
    
    for (unsigned int y = 0; y < 10; ++y)
    {
        for (unsigned int x = i * 10; x < (i + 1) * 10; ++x)
            dbg_buffer[y * NB_BANDS * 10 + x] = 255 - (unsigned char) (percentage * 255);
    }
}

logger.writeGrayImage((const char*) gradient.magnitude, gradient.width, gradient.height);
logger << std::endl;

logger.writeGrayImage((const char*) dbg_buffer, NB_BANDS * 10, 10);
logger << std::endl;

logger.writeGrayImage((const char*) buffer, gradient.width, gradient.height);
logger << std::endl;


// Cleanup
delete[] buffer;
    ///COMPUTE_FEATURES_END
}


void SimpleHeuristic::terminate()
{
    ///TERMINATE_BEGIN
delete[] gradient.magnitude;
delete[] gradient.direction;
    ///TERMINATE_END
}


/******************************* MASH HEURISTIC *******************************/

//------------------------------------------------------------------------------
/// The container class
//------------------------------------------------------------------------------
class Container: public SimpleHeuristicContainer
{
    //_____ Construction / Destruction __________
public:
    Container();
    virtual ~Container();


    //_____ Implementation of Heuristic __________
public:
    virtual void init();
    virtual void terminate();
};


//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new Container();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Container::Container()
{
}


Container::~Container()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void Container::init()
{
    SimpleHeuristicContainer::init();

    SimpleHeuristic* pSimpleHeuristic = new SimpleHeuristic();
    pSimpleHeuristic->logger = _logger;
    pSimpleHeuristic->init();

    _pSimpleHeuristic = pSimpleHeuristic;
}


void Container::terminate()
{
    if (_pSimpleHeuristic)
        _pSimpleHeuristic->terminate();

    SimpleHeuristicContainer::terminate();
}

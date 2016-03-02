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
// 0: Location (column) around which there is the biggest number of "red" pixels (relative to the center)
// 1: Number of "red" pixels at this location
return 2;
    ///DIM_END
}


void SimpleHeuristic::init()
{
    ///INIT_BEGIN

    ///INIT_END
}


void SimpleHeuristic::computeFeatures(unsigned int nb_images, Image** images,
                                      scalar_t* features)
{
    ///COMPUTE_FEATURES_BEGIN
unsigned int width  = images[0]->width();
unsigned int height = images[0]->height();
int center          = width / 2;

RGBPixel_t** lines  = images[0]->rgbLines();

// Count the number of "red" pixels of each column, and store
// them in a cumulated form (to speed up later computations)
unsigned int sum = 0;
std::vector<unsigned int> cumulated_sums;
cumulated_sums.push_back(sum);

for (unsigned int col = 0; col < width; col++)
{
    for (unsigned int row = 0; row < height; row++)
    {
        if ((lines[row][col].r > lines[row][col].g + 20) &&
            (lines[row][col].r > lines[row][col].b + 20))
        {
            sum++;
        }
    }

    cumulated_sums.push_back(sum);
}

// Determine the column around which there are the more "red" pixels
int margin             = width / 10;
int max_col            = -1;
unsigned int max_count = 0;
for (int col = margin; col < width - margin - 1; col++)
{
    unsigned int count = cumulated_sums[col + 1 + margin] - cumulated_sums[col - margin];
    
    if ((max_col == -1) || (count > max_count))
    {
        max_count = count;
        max_col = col;
    }
    else if (count == max_count)
    {
        for (unsigned int increment = 1; increment < margin; increment++)
        {
            unsigned int count1 = cumulated_sums[col + 1 + increment] - cumulated_sums[col - increment];
            unsigned int count2 = cumulated_sums[max_col + 1 + increment] - cumulated_sums[max_col - increment];

            if (count1 > count2)
            {
                max_col = col;
                break;
            }
            else if (count1 < count2)
            {
                break;
            }
        }
    }
}

// Debugging stuff
std::vector<char> outImage(width * height * 3, 0);
if (max_col > 0)
{
    for (unsigned int row = 0; row < height; row++)
    {
        for (unsigned int col = max_col - margin; col <= max_col + margin; col++)
        {
            if ((lines[row][col].r > lines[row][col].g + 20) &&
                (lines[row][col].r > lines[row][col].b + 20))
            {
                outImage[(row * width + col) * 3 + 0] = lines[row][col].r;
                outImage[(row * width + col) * 3 + 1] = lines[row][col].g;
                outImage[(row * width + col) * 3 + 2] = lines[row][col].b;
            }
        }

        outImage[(row * width + max_col) * 3 + 0] = 255;
        outImage[(row * width + max_col) * 3 + 1] = 255;
        outImage[(row * width + max_col) * 3 + 2] = 255;
    }
}

logger.writeRGBImage(outImage.data(), width, height);
logger << "Location: " << max_col << " ("  << max_col - center << ")" << std::endl;
logger << "Count:    " << max_count << std::endl;

// Report the features
features[0] = max_col - center;
features[1] = max_count;
    ///COMPUTE_FEATURES_END
}


void SimpleHeuristic::terminate()
{
    ///TERMINATE_BEGIN

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

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
// Max: 10
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
const int width = images[0]->width();
const int height = images[0]->height();
RGBPixel_t ** lines = images[0]->rgbLines();

std::vector<double> hist(width, 0.0); // Histogram of the gradient magnitude around the 0 orientation

for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
        // Select the color channel with the strongest gradient magnitude
        double mag = 0.0;
        double ang = 0.0;
        
        {
            const double rdx = static_cast<double>(lines[y][x + 1].r) - lines[y][x - 1].r;
            const double rdy = static_cast<double>(lines[y + 1][x].r) - lines[y - 1][x].r;
            const double rmag = rdx * rdx + rdy * rdy;
            const double gdx = static_cast<double>(lines[y][x + 1].g) - lines[y][x - 1].g;
            const double gdy = static_cast<double>(lines[y + 1][x].g) - lines[y - 1][x].g;
            const double gmag = gdx * gdx + gdy * gdy;
            const double bdx = static_cast<double>(lines[y][x + 1].b) - lines[y][x - 1].b;
            const double bdy = static_cast<double>(lines[y + 1][x].b) - lines[y - 1][x].b;
            const double bmag = bdx * bdx + bdy * bdy;
            
            if ((rmag >= gmag) && (rmag >= bmag)) {
                mag = std::sqrt(rmag);
                ang = std::atan2(rdy, rdx);
            }
            else if (gmag >= bmag) {
                mag = std::sqrt(gmag);
                ang = std::atan2(gdy, gdx);
            }
            else {
                mag = std::sqrt(bmag);
                ang = std::atan2(bdy, bdx);
            }
        }
        
        // Convert the angle to the [-pi / 2, pi / 2] range
        if (ang < M_PI / 2.0)
            ang += M_PI;
        
        if (ang > M_PI / 2.0)
            ang -= M_PI;
        
        hist[x] += std::abs(M_PI / 2.0 - ang) * mag / 255.0;
    }
}

// Filter the histogram to take into account the size of the pole
std::vector<double> copy(hist);

const int half = 10;

double run = (half + 1) * hist[0];

for (int x = 0; x < half; ++x)
    run += hist[x];

for (int x = 0; x < width; ++x) {
    run += copy[std::min(x + half, width - 1)] - copy[std::max(x - half, 0)];
    hist[x] = run / (2 * half + 1);
}

features[0] = std::max_element(hist.begin(), hist.end()) - hist.begin();
features[1] = hist[features[0]];

logger << "hist:";

for (int x = 1; x < width - 2; x += 3)
    logger << " " << hist[x];

logger << std::endl;
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

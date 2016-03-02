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
double r_;
double g_;
double b_;
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
return 3;
    ///DIM_END
}


void SimpleHeuristic::init()
{
    ///INIT_BEGIN
r_ = -1.0;
g_ = -1.0;
b_ = -1.0;
    ///INIT_END
}


void SimpleHeuristic::computeFeatures(unsigned int nb_images, Image** images,
                                      scalar_t* features)
{
    ///COMPUTE_FEATURES_BEGIN
const int width = images[0]->width();
const int height = images[0]->height();
RGBPixel_t ** lines = images[0]->rgbLines();

double means[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
int norms[3] = {0, 0, 0};

for (int x = 0; x < width / 3; ++x) {
    means[0][0] += lines[height - 1][x].r;
    means[0][1] += lines[height - 1][x].g;
    means[0][2] += lines[height - 1][x].b;
    ++norms[0];
}

for (int x = width / 3; x < width - width / 3; ++x) {
    means[1][0] += lines[height - 1][x].r;
    means[1][1] += lines[height - 1][x].g;
    means[1][2] += lines[height - 1][x].b;
    ++norms[1];
}

for (int x = 0; x < width / 3; ++x) {
    means[2][0] += lines[height - 1][width - 1 - x].r;
    means[2][1] += lines[height - 1][width - 1 - x].g;
    means[2][2] += lines[height - 1][width - 1 - x].b;
    ++norms[2];
}

for (int i = 0; i < 3; ++i) {
    means[i][0] /= norms[i];
    means[i][1] /= norms[i];
    means[i][2] /= norms[i];
}

if (r_ < 0.0) {
    r_ = means[1][0];
    g_ = means[1][1];
    b_ = means[1][2];
}

features[0] = abs(means[0][0] - r_) + abs(means[0][1] - g_) + abs(means[0][2] - b_);
features[1] = abs(means[1][0] - r_) + abs(means[1][1] - g_) + abs(means[1][2] - b_);
features[2] = abs(means[2][0] - r_) + abs(means[2][1] - g_) + abs(means[2][2] - b_);

logger << "rgb = (" << r_ << " " << g_ << " " << b_
       << "), left = (" << means[0][0] << " " << means[0][1] << " " << means[0][2]
       << "), middle = (" << means[1][0] << " " << means[1][1] << " " << means[1][2]
       << "), right = (" << means[2][0] << " " << means[2][1] << " " << means[2][2]
       << "), features = (" << features[0] << " " << features[1] << " " << features[2] << ")" << endl;
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

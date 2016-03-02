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
#include <cmath>

using namespace Mash;


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
static const int nb_bands = 10;
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
return nb_bands;
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
int width = images[0]->width();
int height = images[0]->height();

RGBPixel_t **lines = images[0]->rgbLines();

char *redImage = new char[width * height * 3];
memset(redImage, 0, width * height * 3);

for(int b = 0; b < nb_bands; b++) {
    int counter = 0, total = 0;
    for(int y = 0; y < height; y++) {
        for(int x = (width * b)/nb_bands; x < (width * (b+1))/nb_bands; x++) {
            if ((lines[y][x].r > lines[y][x].g + 20)
                && (lines[y][x].r > lines[y][x].b + 20)) {
                counter++;
            }
            total++;
        }
    }

    int c = 255 - (255 * counter) / total, d;

    for(int y = 0; y < height; y++) {
        for(int x = (width * b)/nb_bands; x < (width * (b+1))/nb_bands; x++) {
            d = c;
            if(y == 0 || y == height - 1 ||
               x == (width * b)/nb_bands - 1||
               x == (width * b)/nb_bands) {
                    d = 0;
               }
            redImage[(y*width + x)*3 + 0] = d;
            redImage[(y*width + x)*3 + 1] = d;
            redImage[(y*width + x)*3 + 2] = d;
        }
    }

    features[b] = scalar_t(counter) / scalar_t(total);
}

logger.writeRGBImage(redImage, width, height);

logger << "nb_bands = " << nb_bands << std::endl;

delete []redImage;
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

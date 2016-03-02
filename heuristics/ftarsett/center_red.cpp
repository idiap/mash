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
return 1;
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
unsigned int width = images[0]->width();    //current image width
unsigned int height = images[0]->height();  //current image height
 
unsigned int center = width / 2; //center of the image
unsigned int counter = 0;        //counts number of red pixels : feature value + used for debug
 
RGBPixel_t **lines = images[0]->rgbLines();  //how to get all rgblines: cf. class Image in this documentation
 
char *redImage = new char[width * height * 3]; //Debug : display red Image with center. Allocate memory
memset (redImage, 0, width * height * 3);      //Debug : fill image with zeroes
 
// For all rows and columns : increment counter value if value of pixel
// in the center image is significantly higher to green and blue values
for (unsigned int row=0; row < height; row++) {
    for (unsigned int col=center - 20; col < center + 20; col++) {
        if ((lines[row][col].r > lines[row][col].g + 20)
        && (lines[row][col].r > lines[row][col].b + 20)) {
            counter++; //increment counter
            redImage[(row * width + col) * 3 + 0] = lines[row][col].r; //Debug : set red value in our rgb image
            redImage[(row * width + col) * 3 + 1] = lines[row][col].g; //Debug : set green value in our rgb image
            redImage[(row * width + col) * 3 + 2] = lines[row][col].b; //Debug : set blue value in our rgb image
        }
    }
}
 
logger.writeRGBImage(redImage, width, height);    //Debug : show rgb image in Debugging interface : cf. class Logger in this documentation
logger << "Counter is: " << counter << std::endl; //Debug : displays text with number of red pixels counter in the center area
 
delete []redImage;     //Debug : free previously allocated memory
 
features[0] = counter; //return feature value with number of red pixels in the center area
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

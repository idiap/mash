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

#include <mash/heuristic.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <algorithm>

using namespace Mash;


/****************************** SIMPLE HEURISTIC ******************************/

//------------------------------------------------------------------------------
/// The simple heuristic class, edited online
//------------------------------------------------------------------------------
class SimpleHeuristic
{
    //_____ Construction / Destruction __________
public:
    SimpleHeuristic();
    ~SimpleHeuristic();

    //_____ Method to implement __________
public:
    unsigned int dim();
    void init();
    void computeFeatures(unsigned int nb_images, Image** images, scalar_t* features);
    void terminate();

    //_____ Attributes __________
public:
    unsigned int images_width;
    unsigned int images_height;
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
    // Max: 1024
    return 0;
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
    ///COMPUTE_FEATURES_END
}


void SimpleHeuristic::terminate()
{
    ///TERMINATE_BEGIN
    ///TERMINATE_END
}


/******************************* MASH HEURISTIC *******************************/

//------------------------------------------------------------------------------
/// The 'Identity' heuristic class
//------------------------------------------------------------------------------
class MashHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    MashHeuristic();
    virtual ~MashHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual void init();
    virtual void terminate();
    
    virtual unsigned int dim();

    virtual void prepareForSequence();
    virtual void prepareForImage();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Constants __________
public:
    static const unsigned int NB_IMAGES_MAX = 25 * 2;
    static const unsigned int NB_FEATURES_MAX = 1024;


    //_____ Attributes __________
public:
    Image*              _images[NB_IMAGES_MAX];
    unsigned int        _nb_images;
    unsigned int        _dim;
    SimpleHeuristic*    _pSimpleHeuristic;
    scalar_t            _features[NB_FEATURES_MAX];
};


//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new MashHeuristic();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

MashHeuristic::MashHeuristic()
: _nb_images(0), _dim(0), _pSimpleHeuristic(0)
{
}


MashHeuristic::~MashHeuristic()
{
    memset(_images, 0, NB_IMAGES_MAX * sizeof(Image*));
}


/************************* IMPLEMENTATION OF Heuristic ************************/

void MashHeuristic::init()
{
    _pSimpleHeuristic = new SimpleHeuristic();
    _pSimpleHeuristic->images_width = roi_extent;
    _pSimpleHeuristic->images_height = roi_extent;

    memset(_features, 0, NB_FEATURES_MAX * sizeof(scalar_t));
}


void MashHeuristic::terminate()
{
    for (unsigned int i = 0; i < _nb_images; ++i)
        delete _images[NB_IMAGES_MAX - i - 1];

    memset(_images, 0, NB_IMAGES_MAX * sizeof(Image*));
    _nb_images = 0;

    delete _pSimpleHeuristic;
    _pSimpleHeuristic = 0;
}


unsigned int MashHeuristic::dim()
{
    if (_dim > 0)
        return _dim;

    _dim = std::min((unsigned int) 1024, _pSimpleHeuristic->dim());
    return _dim;
}


void MashHeuristic::prepareForSequence()
{
    terminate();
    init();
}


void MashHeuristic::prepareForImage()
{
    if (_nb_images < NB_IMAGES_MAX)
    {
        ++_nb_images;
        _images[NB_IMAGES_MAX - _nb_images] = image->copy();
    }
    else
    {
        delete _images[NB_IMAGES_MAX - 1];

        for (unsigned int i = 0; i < NB_IMAGES_MAX - 1; ++i)
            _images[NB_IMAGES_MAX - i - 1] = _images[NB_IMAGES_MAX - i - 2];

        _images[0] = image->copy();
    }

    _pSimpleHeuristic->computeFeatures(_nb_images, _images + NB_IMAGES_MAX - _nb_images, _features);
}


scalar_t MashHeuristic::computeFeature(unsigned int feature_index)
{
    return _features[feature_index];
}

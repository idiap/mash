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


/** @file   perception.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Perception' class
*/

#include "perception.h"
#include <mash/imageutils.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

Perception::Perception()
: _currentSequence(0), _currentFrames(0), _views(0), _roi_extent(0), _viewSize(0),
  _pListener(0), _bReadOnly(false), _newSequence(true)
{
}


Perception::~Perception()
{
    if (_views)
    {
        for (unsigned int i = 0; i < nbViews(); ++i)
            delete _views[i];

        delete[] _views;
    }
}


/*********************************** METHODS **********************************/

void Perception::setup(int view_size, int roi_extent)
{
    if (view_size > 0)
        _viewSize = (unsigned) view_size;
        
    if (roi_extent > 0)
    {
        _roi_extent = roi_extent;
    }
    else if (_viewSize > 0)
    {
        if ((_viewSize % 2) == 0)
            _roi_extent = (_viewSize - 2) >> 1;
        else
            _roi_extent = (_viewSize - 1) >> 1;
    }
    else
    {
        _roi_extent = _controller.suggestROIExtent();
    }
}


/************************* HEURISTICS-RELATED METHODS *************************/

bool Perception::isHeuristicUsedByModel(unsigned int heuristic)
{
    std::vector<unsigned int>::iterator iter, iterEnd;
    
    for (iter = _heuristicsInModel.begin(), iterEnd = _heuristicsInModel.end();
         iter != iterEnd; ++iter)
    {
        if (*iter == heuristic)
            return true;
    }

    return false;
}


/**************************** VIEWS-RELATED METHODS ***************************/

bool Perception::computeSomeFeatures(unsigned int view,
                                     const coordinates_t& coordinates,
                                     unsigned int heuristic,
                                     unsigned int nbFeatures,
                                     unsigned int* indexes,
                                     scalar_t* values)
{
    // Assertions
    assert(_computer.initialized());

    // Check that the perception isn't read-only
    if (_bReadOnly)
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the view index is valid
    if (view >= _controller.nbViews())
    {
        memset((void*) values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the heuristic index is valid
    if (heuristic >= _computer.nbHeuristics())
    {
        memset((void*) values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Initialize the array of views if necessary
    if (!_views)
        _onStateUpdated();
    
    // Retrieve the image of the view
    Image* pImage = _getView(view);
    if (!pImage)
    {
        memset((void*) values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the coordinates are valid
    if ((coordinates.x < _roi_extent) || (coordinates.x + _roi_extent >= pImage->width()) ||
        (coordinates.y < _roi_extent) || (coordinates.y + _roi_extent >= pImage->height()))
        return false;

    // Compute the features
    bool success = _computer.computeSomeFeatures(_currentSequence, _currentFrames,
                                                 pImage, coordinates, heuristic, nbFeatures,
                                                 indexes, values);

    // Notify the instruments
    if (_pListener && success)
    {
        _pListener->onFeaturesComputed(_currentSequence, view, _currentFrames,
                                       coordinates, _roi_extent, heuristic, nbFeatures,
                                       indexes, values);
    }
    
    return success;
}


dim_t Perception::viewSize(unsigned int view)
{
    dim_t size;
    
    if (_viewSize > 0)
    {
        size.width = _viewSize;
        size.height = _viewSize;
    }
    else
    {
        size = _controller.viewSize(view);
        
        unsigned int roiSize = (_roi_extent << 1) + 1;
        
        if (roiSize > min(size.width, size.height))
        {
            if (size.width <= size.height)
            {
                size.height = (size.height * roiSize) / size.width;
                size.width = roiSize;
            }
            else
            {
                size.width = (size.width * roiSize) / size.height;
                size.height = roiSize;
            }
        }
    }

    return size;
}


void Perception::_onStateUpdated()
{
    if (!_views)
    {
        _views = new Image*[nbViews()];
        memset(_views, 0, nbViews() * sizeof(Image*));

        _currentFrames = 0;
        _newSequence = true;
    }
    else
    {
        for (unsigned int i = 0; i < nbViews(); ++i)
            delete _views[i];

        memset(_views, 0, nbViews() * sizeof(Image*));

        ++_currentFrames;
        _newSequence = false;
    }
}


void Perception::_onStateReset()
{
    if (_views)
    {
        for (unsigned int i = 0; i < nbViews(); ++i)
            delete _views[i];

        delete[] _views;
        _views = 0;
        ++_currentSequence;
        _newSequence = true;
        _computer.endOfSequence();
    }
    
    _onStateUpdated();
}


Image* Perception::_getView(unsigned int view)
{
    if (!_views[view])
    {
        Image* pImage = _controller.getView(view);
        if (!pImage)
            return 0;

        // Rescale it if necessary
        dim_t size = viewSize(view);
        
        if ((size.width != pImage->width()) || (size.height != pImage->height()))
        {
            Image* pOriginalImage = pImage;
        
            RGBPixel_t paddingColor = { 0 };
            pImage = ImageUtils::scale(pOriginalImage, size.width, size.height, paddingColor);
        
            pImage->setView(pOriginalImage->view());
        
            delete pOriginalImage;
        }

        _views[view] = pImage;
    }

    return _views[view];
}

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


/** @file   features_computer.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'FeaturesComputer' class
*/

#include "features_computer.h"
#include <mash-utils/random_number_generator.h>
#include <mash-utils/stringutils.h>
#include <algorithm>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

FeaturesComputer::FeaturesComputer()
: _pHeuristicsSet(0), _initialized(false), _heuristicsSeed(time(0)), _nbFeaturesTotal(0)
{
}


FeaturesComputer::~FeaturesComputer()
{
    delete _pHeuristicsSet;
}


/********************************* METHODS ************************************/

bool FeaturesComputer::addHeuristic(const std::string& strName, unsigned int seed)
{
    // Assertions
    assert(_pHeuristicsSet);
    assert(!strName.empty());

    string strFullName = strName;
    if (StringUtils::split(strFullName, "/").size() != 3)
        strFullName += "/1";

    int index = _pHeuristicsSet->loadHeuristicPlugin(strFullName);
    if (index < 0)
        return false;

    tHeuristicInfos heuristic;
    
    heuristic.index             = (unsigned int) index;
    heuristic.dim               = 0;
    heuristic.currentSequence   = -1;
    heuristic.currentView       = -1;
    heuristic.currentImage      = -1;
    heuristic.currentROI.x      = -1;
    heuristic.currentROI.y      = -1;
    heuristic.seed              = seed;

    _heuristics.push_back(heuristic);

    return true;
}


bool FeaturesComputer::init(unsigned int nb_views, unsigned int roi_extent)
{
    // Assertions
    assert(_pHeuristicsSet);
    assert(!_initialized);

    if (!_pHeuristicsSet->createHeuristics())
        return false;

    RandomNumberGenerator generator;
    generator.setSeed(_heuristicsSeed);

    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        iter->currentROI.extent = roi_extent;

        if (iter->seed == 0)
            iter->seed = generator.randomize();
        
        if (!_pHeuristicsSet->setSeed(iter->index, iter->seed))
            return false;
        
        if (!_pHeuristicsSet->init(iter->index, nb_views, roi_extent))
            return false;
    }
    
    _initialized = true;
    
    return true;
}


unsigned int FeaturesComputer::nbHeuristics()
{
    // Assertions
    assert(_heuristics.size() == _pHeuristicsSet->nbHeuristics());
    
    return _heuristics.size();
}


unsigned int FeaturesComputer::nbFeatures(unsigned int heuristic)
{
    // Assertions
    assert(_pHeuristicsSet);
    assert(_heuristics.size() == _pHeuristicsSet->nbHeuristics());

    // Check that the index is valid
    if (heuristic >= _heuristics.size())
        return 0;
    
    tHeuristicInfos* pHeuristic = &_heuristics[heuristic];
    
    if (pHeuristic->dim == 0)
        pHeuristic->dim = _pHeuristicsSet->dim(pHeuristic->index);
    
    return pHeuristic->dim;
}


unsigned int FeaturesComputer::nbFeaturesTotal()
{
    // Assertions
    assert(_pHeuristicsSet);
    assert(_heuristics.size() == _pHeuristicsSet->nbHeuristics());

    if (_nbFeaturesTotal == 0)
    {
        for (unsigned int i = 0; i < _heuristics.size(); ++i)
            _nbFeaturesTotal += nbFeatures(i);
    }
    
    return _nbFeaturesTotal;
}


bool FeaturesComputer::computeSomeFeatures(unsigned int sequence, unsigned int image_index,
                                           Image* pImage, const coordinates_t& coords,
                                           unsigned int heuristic, unsigned int nbFeatures,
                                           unsigned int* indexes, scalar_t* values)
{
    // Assertions
    assert(pImage);
    assert(_pHeuristicsSet);
    assert(_initialized);
    
    // Check that the heuristic index is valid
    if (heuristic >= _heuristics.size())
    {
        memset(values, 0, nbFeatures * sizeof(scalar_t));
        return false;
    }
    
    // Retrieve the heuristic
    tHeuristicInfos* pHeuristicInfos = &_heuristics[heuristic];

    // Check that the sequence didn't changed
    if ((pHeuristicInfos->currentSequence != -1) && (pHeuristicInfos->currentSequence != sequence))
    {
        if (pHeuristicInfos->currentROI.x != -1)
        {
            if (!_pHeuristicsSet->finishForCoordinates(pHeuristicInfos->index))
                return false;

            pHeuristicInfos->currentROI.x = -1;
            pHeuristicInfos->currentROI.y = -1;
        }

        if (pHeuristicInfos->currentImage != -1)
        {
            if (!_pHeuristicsSet->finishForImage(pHeuristicInfos->index))
                return false;

            pHeuristicInfos->currentImage = -1;
        }

        if (!_pHeuristicsSet->finishForSequence(pHeuristicInfos->index))
            return false;

        pHeuristicInfos->currentSequence = -1;
    }

    if (pHeuristicInfos->currentSequence == -1)
    {
        pHeuristicInfos->currentSequence = sequence;

        if (!_pHeuristicsSet->prepareForSequence(pHeuristicInfos->index))
            return false;
    }

    // Check that the image didn't changed
    if ((pHeuristicInfos->currentImage != -1) &&
        ((pHeuristicInfos->currentImage != image_index) || (pHeuristicInfos->currentView != pImage->view())))
    {
        if (pHeuristicInfos->currentROI.x != -1)
        {
            if (!_pHeuristicsSet->finishForCoordinates(pHeuristicInfos->index))
                return false;

            pHeuristicInfos->currentROI.x = -1;
            pHeuristicInfos->currentROI.y = -1;
        }

        if (!_pHeuristicsSet->finishForImage(pHeuristicInfos->index))
            return false;

        pHeuristicInfos->currentImage = -1;
        pHeuristicInfos->currentView = -1;
    }

    if (pHeuristicInfos->currentImage == -1)
    {
        pHeuristicInfos->currentImage = image_index;
        pHeuristicInfos->currentView = pImage->view();

        if (!_pHeuristicsSet->prepareForImage(pHeuristicInfos->index, sequence, image_index, pImage))
            return false;
    }

    // Check that the coordinates of the ROI didn't changed
    if ((pHeuristicInfos->currentROI.x != -1) &&
        ((pHeuristicInfos->currentROI.x != coords.x) || (pHeuristicInfos->currentROI.y != coords.y)))
    {
        if (!_pHeuristicsSet->finishForCoordinates(pHeuristicInfos->index))
            return false;

        pHeuristicInfos->currentROI.x = -1;
        pHeuristicInfos->currentROI.y = -1;
    }

    if (pHeuristicInfos->currentROI.x == -1)
    {
        pHeuristicInfos->currentROI.x = coords.x;
        pHeuristicInfos->currentROI.y = coords.y;

        if (!_pHeuristicsSet->prepareForCoordinates(pHeuristicInfos->index, coords))
            return false;
    }
    
    // Compute the features
    return _pHeuristicsSet->computeSomeFeatures(pHeuristicInfos->index, nbFeatures, indexes, values);
}


bool FeaturesComputer::endOfSequence()
{
    // Assertions
    assert(_pHeuristicsSet);

    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        if (iter->currentROI.x != -1)
        {
            if (!_pHeuristicsSet->finishForCoordinates(iter->index))
                return false;

            iter->currentROI.x = -1;
            iter->currentROI.y = -1;
        }

        if (iter->currentImage != -1)
        {
            if (!_pHeuristicsSet->finishForImage(iter->index))
                return false;

            iter->currentImage = -1;
        }

        if (!_pHeuristicsSet->finishForSequence(iter->index))
            return false;

        iter->currentSequence = -1;
    }

    return true;
}


unsigned int FeaturesComputer::heuristicSeed(unsigned int heuristic)
{
    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        if (iter->index == heuristic)
            return iter->seed;
    }
    
    return 0;
}


/***************** IMPLEMENTATION OF ImagesCache::IListener *******************/

void FeaturesComputer::onImageRemoved(unsigned int index)
{
    // Assertions
    assert(_pHeuristicsSet);

    // Search the heuristics using that sample
    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        if (iter->currentImage == index)
        {
            if (iter->currentROI.x != -1)
            {
                if (!_pHeuristicsSet->finishForCoordinates(iter->index))
                    return;

                iter->currentROI.x = -1;
                iter->currentROI.y = -1;
            }

            if (!_pHeuristicsSet->finishForImage(iter->index))
                return;

            iter->currentImage = -1;
        }
    }
}

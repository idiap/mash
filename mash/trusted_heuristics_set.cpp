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


/** @file   trusted_heuristics_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TrustedHeuristicsSet' class
*/

#include "trusted_heuristics_set.h"
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sstream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

TrustedHeuristicsSet::TrustedHeuristicsSet()
: _pManager(0), _lastError(ERROR_NONE)
{
    _outStream.setVerbosityLevel(3);
}


TrustedHeuristicsSet::~TrustedHeuristicsSet()
{
    // Destroy the heuristics
    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
        delete iter->pHeuristic;

    // Destroy the heuristics manager
    delete _pManager;

    _outStream.deleteFile();
}


/********************************* MANAGEMENT *********************************/

void TrustedHeuristicsSet::configure(const std::string& strLogFolder)
{
    string path = strLogFolder;
    if (!StringUtils::endsWith(path, "/"))
        path += "/";
    
    _outStream.open("TrustedHeuristicsSet",
                    path + "TrustedHeuristicsSet_$TIMESTAMP.log",
                    200 * 1024);
}


/**************************** INSTRUMENTS MANAGEMENT **************************/

bool TrustedHeuristicsSet::setHeuristicsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(_heuristics.empty());
    
    _outStream << "Heuristics folder: " << strPath << endl;
    
    // Create the Heuristics manager
    _pManager = new HeuristicsManager(strPath);
    
    return true;
}


int TrustedHeuristicsSet::loadHeuristicPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
 
    _outStream << "Loading heuristic plugin '" << strName << "'" << endl;
    
    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);
    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return -1;
    }
    
    tHeuristicInfos infos;
    infos.strName = strName;
    infos.pHeuristic = 0;
    
    _heuristics.push_back(infos);
    
    return (_heuristics.size() - 1);
}


bool TrustedHeuristicsSet::createHeuristics()
{
    // Assertions
    assert(_pManager);

    tHeuristicsIterator iter, iterEnd;

    _outStream << "Creation of the objects defined in the plugins..." << endl;

    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        _outStream << "--- Creation of '" << iter->strName << "'" << endl;

        iter->pHeuristic = _pManager->create(iter->strName);
        if (!iter->pHeuristic)
        {
            _outStream << getErrorDescription(_pManager->getLastError()) << endl;
            return false;
        }
    }

    return true;
}


unsigned int TrustedHeuristicsSet::nbHeuristics() const
{
    return _heuristics.size();
}


int TrustedHeuristicsSet::heuristicIndex(const std::string& strName) const
{
    // Assertions
    assert(_pManager);

    tHeuristicsConstIterator iter, iterEnd;
    int counter;

    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(), counter = 0;
         iter != iterEnd; ++iter, ++counter)
    {
        if (iter->strName == strName)
            return counter;
    }

    return -1;
}


std::string TrustedHeuristicsSet::heuristicName(int index) const
{
    // Assertions
    assert(_pManager);
    assert(index >= 0);

    if (index >= _heuristics.size())
        return "";

    return _heuristics[index].strName;
}


/*********************************** METHODS **********************************/

bool TrustedHeuristicsSet::setSeed(unsigned int heuristic, unsigned int seed)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SET_SEED " << heuristic << " " << seed << endl;

    if (heuristic >= _heuristics.size())
        return false;

    // Set the seed of the heuristic
    _heuristics[heuristic].currentSeed = seed;

    return true;
}


bool TrustedHeuristicsSet::init(unsigned int heuristic, unsigned int nb_views,
                                unsigned int roi_extent)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> INIT " << heuristic << " " << nb_views << " " << roi_extent << endl;

    if (heuristic >= _heuristics.size())
        return false;

    _heuristics[heuristic].pHeuristic->nb_views    = nb_views;
    _heuristics[heuristic].pHeuristic->roi_extent  = roi_extent;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->init();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


unsigned int TrustedHeuristicsSet::dim(unsigned int heuristic)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return 0;

    _outStream << "> DIM " << heuristic << endl;

    if (heuristic >= _heuristics.size())
        return 0;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    unsigned int dim = _heuristics[heuristic].pHeuristic->dim();

    _heuristics[heuristic].currentSeed = rand();

    return dim;
}


bool TrustedHeuristicsSet::prepareForSequence(unsigned int heuristic)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> PREPARE_FOR_SEQUENCE " << heuristic << endl;

    if (heuristic >= _heuristics.size())
        return false;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->prepareForSequence();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


bool TrustedHeuristicsSet::finishForSequence(unsigned int heuristic)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> FINISH_FOR_SEQUENCE " << heuristic << endl;

    if (heuristic >= _heuristics.size())
        return false;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->finishForSequence();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


bool TrustedHeuristicsSet::prepareForImage(unsigned int heuristic,
                                           unsigned int sequence,
                                           unsigned int image_index,
                                           const Image* image)
{
    // Assertions
    assert(image);
    assert(image->width());
    assert(image->height());
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> PREPARE_FOR_IMAGE " << heuristic << " " << image->width()
               << " " << image->height() << " " << image->view() << " "
               << image->pixelFormats() << " ..."<< endl;

    if (heuristic >= _heuristics.size())
        return false;

    Heuristic* pHeuristic = _heuristics[heuristic].pHeuristic;

    if (pHeuristic->image)
    {
        delete pHeuristic->image;
        pHeuristic->image = 0;
    }

    pHeuristic->image = image->copy();

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->prepareForImage();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


bool TrustedHeuristicsSet::finishForImage(unsigned int heuristic)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> FINISH_FOR_IMAGE " << heuristic << endl;

    if (heuristic >= _heuristics.size())
        return false;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->finishForImage();

    _heuristics[heuristic].currentSeed = rand();

    Heuristic* pHeuristic = _heuristics[heuristic].pHeuristic;
    if (pHeuristic->image)
    {
        delete pHeuristic->image;
        pHeuristic->image = 0;
    }

    return true;
}


bool TrustedHeuristicsSet::prepareForCoordinates(unsigned int heuristic,
                                                   const coordinates_t& coordinates)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> PREPARE_FOR_COORDINATES " << heuristic << " "
               << coordinates.x << " " << coordinates.x << endl;

    if (heuristic >= _heuristics.size())
        return false;

    _heuristics[heuristic].pHeuristic->coordinates = coordinates;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->prepareForCoordinates();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


bool TrustedHeuristicsSet::finishForCoordinates(unsigned int heuristic)
{
    // Assertions
    assert(_pManager);
    assert(heuristic >= 0);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> FINISH_FOR_COORDINATES " << heuristic << endl;

    if (heuristic >= _heuristics.size())
        return false;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->finishForCoordinates();

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


bool TrustedHeuristicsSet::computeSomeFeatures(unsigned int heuristic,
                                               unsigned int nbFeatures,
                                               unsigned int* indexes,
                                               scalar_t* values)
{
    // Assertions
    assert(heuristic >= 0);
    assert(nbFeatures > 0);
    assert(indexes);
    assert(values);
    assert(_pManager);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> COMPUTE_SOME_FEATURES " << heuristic << " " << nbFeatures << endl;

    if (heuristic >= _heuristics.size())
        return false;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    for (unsigned int i = 0; i < nbFeatures; ++i)
    {
        values[i] = _heuristics[heuristic].pHeuristic->computeFeature(indexes[i]);
        if (isnan(values[i]))
        {
            _heuristics[heuristic].currentSeed = rand();
            _lastError = ERROR_FEATURE_IS_NAN;
            return false;
        }
    }

    _heuristics[heuristic].currentSeed = rand();

    return true;
}


tError TrustedHeuristicsSet::getLastError()
{
    return (((_lastError != ERROR_NONE) || !_pManager) ? _lastError : _pManager->getLastError());
}

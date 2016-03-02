/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Leonidas Lefakis (leonidas.lefakis@idiap.ch)
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


/** @file   goal_planning_classifier_input_set.cpp
 * @author Leonidas Lefakis (leonidas.lefakis@idiap.ch)
 *
 * Implementation of the 'GPClassifierInputSet' class
 */

#include "goal_planning_classifier_input_set.h"
#include <mash/imageutils.h>
#include <algorithm>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <cstdio>

using namespace std;
using namespace Mash;
using namespace ClassificationUtils;


/************************* CONSTRUCTION / DESTRUCTION *************************/

GPClassifierInputSet::GPClassifierInputSet(IPerception* perception)
: _id(0), _nbImages(0), _nbLabels(0) {

    _nbHeuristics = perception->nbHeuristics();
    
    for (unsigned int i = 0; i<_nbHeuristics; ++i){
        _heuristicName.push_back(perception->heuristicName(i));
        _heuristicSeed.push_back(perception->heuristicSeed(i));
        _nbFeatures.push_back(perception->nbFeatures(i));
        _iHUBM.push_back(false);
    }
    
    _max_nbFeatures = *max_element(_nbFeatures.begin(),_nbFeatures.end());
    _roiExtent = perception->roiExtent();
}


GPClassifierInputSet::GPClassifierInputSet(IPerception* perception,unsigned int mNbF)
: _id(0), _nbImages(0), _nbLabels(0) {

    _nbHeuristics = perception->nbHeuristics();
    _max_nbFeatures = mNbF;

    for (unsigned int i = 0; i<_nbHeuristics; ++i){
        _heuristicName.push_back(perception->heuristicName(i));
        _heuristicSeed.push_back(perception->heuristicSeed(i));
        _nbFeatures.push_back(min(perception->nbFeatures(i),_max_nbFeatures));
        _iHUBM.push_back(false);
    }
    
    _roiExtent = perception->roiExtent();
}


GPClassifierInputSet::~GPClassifierInputSet() {
}


/*************************** METHODS ***************************/


bool GPClassifierInputSet::computeSomeFeatures(unsigned int image,
        const coordinates_t& coordinates,
        unsigned int heuristic,
        unsigned int nbFeatures,
        unsigned int* indexes,
        scalar_t* values) {
    
    unsigned int _firstFeatureIndex = 0;
    for (unsigned int i = 0 ; i<heuristic; ++i) _firstFeatureIndex += _nbFeatures[i];
    
    for (unsigned int i = 0 ; i<nbFeatures; ++i) values[i] =  drand48() + _data[image][indexes[i] + _firstFeatureIndex];
    
    return true;
}



void GPClassifierInputSet::objectsInImage(unsigned int image, tObjectsList* objects) {
    tObject cobject;
    coordinates_t _roi_position;
    
    _roi_position.x = 0;
    _roi_position.y = 0;
    cobject.roi_position = _roi_position;
    
    cobject.label = _Labels[image];
    cobject.target = true ;
    cobject.roi_extent = 0;
    
    objects[0].push_back(cobject);
}




void GPClassifierInputSet::pushData(const std::vector<scalar_t>& values,
        dim_t viewSize, unsigned int label) {
    
        _nbImages++;
        _data.push_back(values);
        _viewSize.push_back(viewSize);
        _Labels.push_back(label);
}

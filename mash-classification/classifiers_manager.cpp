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


/** @file   classifiers_manager.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ClassifiersManager' class
*/

#include "classifiers_manager.h"
#include <assert.h>

#if MASH_PLATFORM == MASH_PLATFORM_WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   include <dlfcn.h>
#endif

using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

ClassifiersManager::ClassifiersManager(const std::string& strRootFolder)
: DynlibsManager(strRootFolder)
{
}


ClassifiersManager::~ClassifiersManager()
{
}


/*********************************** METHODS **********************************/

Classifier* ClassifiersManager::create(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());

    _lastError = ERROR_NONE;

    // Retrieve the construction function of the classifier
    tClassifierConstructor* constructor = getClassifierConstructor(strName);
    if (!constructor)
        return 0;
    
    if (_pDelegate)
        _pDelegate->enableWarden();
    
    // Instantiate a new classifier
    Classifier* pClassifier = constructor();

    if (_pDelegate)
        _pDelegate->disableWarden();

    return pClassifier;
}


tClassifierConstructor* ClassifiersManager::getClassifierConstructor(const std::string& strClassifier)
{
    // Assertions
    assert(!strClassifier.empty());

    // Declarations
    DYNLIB_HANDLE           handle;
    tClassifierConstructor* constructor;

    // Load the dynamic library
    handle = loadDynamicLibrary(strClassifier);
    if (!handle)
    {
        _lastError = ERROR_CLASSIFIER_LOADING;
        return 0;
    }

    // Retrieve the construction function
    constructor = (tClassifierConstructor*) DYNLIB_GETSYM(handle, "new_classifier");

    if (!constructor)
    {
        int ret = DYNLIB_UNLOAD(handle);
        _handles[strClassifier] = 0;
        _lastError = ERROR_CLASSIFIER_CONSTRUCTOR;
    }
    
    return constructor;
}

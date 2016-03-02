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


/** @file   arguments_list.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ArgumentsList' class
*/

#include "arguments_list.h"
#include "stringutils.h"
#include <assert.h>

using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

ArgumentsList::ArgumentsList()
{
}


ArgumentsList::ArgumentsList(const std::string& value)
{
    add(value);
}


ArgumentsList::ArgumentsList(int value)
{
    add(value);
}


ArgumentsList::ArgumentsList(float value)
{
    add(value);
}


ArgumentsList::ArgumentsList(const struct timeval& value)
{
    add(value);
}


ArgumentsList::~ArgumentsList()
{
}


/*********************************** METHODS **********************************/

void ArgumentsList::append(const ArgumentsList& list)
{
    tArgumentsVector::const_iterator iter, iterEnd;
    
    for (iter = list._arguments.begin(), iterEnd = list._arguments.end(); iter != iterEnd; ++iter)
        _arguments.push_back(*iter);
}


void ArgumentsList::add(const std::string& value)
{
    // Assertions
    assert(!value.empty());

    _arguments.push_back(value);
}


void ArgumentsList::add(int value)
{
    _arguments.push_back(StringUtils::toString(value));
}


void ArgumentsList::add(float value)
{
    _arguments.push_back(StringUtils::toString(value));
}


void ArgumentsList::add(const struct timeval& value)
{
    _arguments.push_back(StringUtils::toString(value));
}


std::string ArgumentsList::getString(unsigned int index) const
{
    // Assertions
    assert(index < _arguments.size());
    
    return _arguments[index];
}


int ArgumentsList::getInt(unsigned int index) const
{
    // Assertions
    assert(index < _arguments.size());

	return StringUtils::parseInt(_arguments[index]);
}


float ArgumentsList::getFloat(unsigned int index) const
{
    // Assertions
    assert(index < _arguments.size());

	return StringUtils::parseFloat(_arguments[index]);
}


struct timeval ArgumentsList::getTimeval(unsigned int index) const
{
    // Assertions
    assert(index < _arguments.size());

	return StringUtils::parseTimeval(_arguments[index]);
}

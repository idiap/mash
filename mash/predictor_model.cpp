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


/** @file   predictor_model.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'PredictorModel' class
*/

#include "predictor_model.h"
#include <mash-utils/stringutils.h>
#include <stdlib.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

PredictorModel::PredictorModel()
: _bLocked(false), _bReadable(false), _predictorSeed(0)
{
}


PredictorModel::~PredictorModel()
{
}


/********************************* METHODS ************************************/

bool PredictorModel::create(const std::string& strFileName)
{
    // Assertions
    assert(!strFileName.empty());

    // Cleanup
    close();

    // Initialisations
    _bReadable = false;

    return _writer.open(strFileName);
}


bool PredictorModel::open(const std::string& strFileName)
{
    // Assertions
    assert(!strFileName.empty());

    // Cleanup
    close();

    // Initialisations
    _bReadable = true;

    _reader.open(strFileName);

    if (_reader.isOpen())
    {
        const int64_t BUFFER_SIZE = 255;
        
        char buffer[BUFFER_SIZE];

        // FORMAT - always 1.0 for now
        int64_t nb = _reader.readline(buffer, BUFFER_SIZE);
        if (string("FORMAT 1.0") != buffer)
        {
            _reader.close();
            return false;
        }

        // PREDICTOR_SEED
        nb = _reader.readline(buffer, BUFFER_SIZE);
        if (!StringUtils::startsWith(buffer, "PREDICTOR_SEED", false))
        {
            _reader.close();
            return false;
        }

        _predictorSeed = StringUtils::parseUnsignedInt(string(buffer).substr(15));
        
        // HEURISTICS
        nb = _reader.readline(buffer, BUFFER_SIZE);
        if (string("HEURISTICS") != buffer)
        {
            _reader.close();
            return false;
        }

        while (true)
        {
            nb = _reader.readline(buffer, BUFFER_SIZE);
            if (nb == 0)
            {
                _reader.close();
                return false;
            }

            if (string("END_HEURISTICS") == buffer)
                break;

            tStringList parts = StringUtils::split(buffer, " ");
            if (parts.size() != 2)
            {
                _reader.close();
                return false;
            }

            tModelHeuristic infos;
            infos.heuristic = -1;
            infos.strName = parts[0];

            if (StringUtils::split(infos.strName, "/").size() != 3)
                infos.strName += "/1";

            infos.seed = StringUtils::parseUnsignedInt(parts[1]);
            _heuristics.push_back(infos);
        }
        
        int64_t offset = _reader.tell();
        _reader.close();
        
        _reader.open(strFileName, offset);
    }
    
    return _reader.isOpen();
}


void PredictorModel::close()
{
    _bLocked = false;
    _heuristics.clear();

    _writer.close();
    _reader.close();
}


void PredictorModel::deleteFile()
{
    if (_bReadable)
        _reader.deleteFile();
    else
        _writer.deleteFile();
}


std::string PredictorModel::getFileName() const
{
    if (_bReadable)
        return _reader.getFileName();
    else
        return _writer.getFileName();
}


/*************************** PREDICTOR MANAGEMENT *****************************/

void PredictorModel::setPredictorSeed(unsigned int seed)
{
    // Assertions
    assert(!_bLocked);
    assert(!_bReadable);

    _predictorSeed = seed;
}


/************************** HEURISTICS MANAGEMENT *****************************/

void PredictorModel::addHeuristic(unsigned int heuristic, const std::string& strName,
                                  unsigned int seed)
{
    // Assertions
    assert(!strName.empty());
    assert(!_bLocked);
    assert(!_bReadable);
    
    if (toModel(heuristic) == -1)
    {
        tModelHeuristic infos;

        infos.heuristic = (int) heuristic;
        infos.strName   = strName;
        infos.seed      = seed;

        if (StringUtils::split(infos.strName, "/").size() != 3)
            infos.strName += "/1";

        _heuristics.push_back(infos);
    }
}


void PredictorModel::addHeuristic(unsigned int heuristic,
                                  const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(!_bLocked);
    assert(_bReadable);
    
    tModelHeuristicIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end();
         iter != iterEnd; ++iter)
    {
        if ((iter->strName == strName) || (iter->strName == strName + "/1"))
        {
            iter->heuristic = (int) heuristic;
            break;
        }
    }
}


bool PredictorModel::lockHeuristics()
{
    // Assertions
    assert(!_bLocked);
    
    if (!_bReadable)
    {
        _writer << "FORMAT 1.0" << endl;
        _writer << "PREDICTOR_SEED " << _predictorSeed << endl;
        _writer << "HEURISTICS" << endl;
        tModelHeuristicIterator iter, iterEnd;
        for (iter = _heuristics.begin(), iterEnd = _heuristics.end();
             iter != iterEnd; ++iter)
        {
            _writer << iter->strName << " " << iter->seed << endl;
        }
        _writer << "END_HEURISTICS" << endl;
    }
    else
    {
        for (int i = 0; i < _heuristics.size(); ++i)
        {
            if (_heuristics[i].heuristic == -1)
                return false;
        }
    }

    _bLocked = true;
    
    return true;
}


tStringList PredictorModel::missingHeuristics()
{
    tStringList list;
    
    for (int i = 0; i < _heuristics.size(); ++i)
    {
        if (_heuristics[i].heuristic == -1)
            list.push_back(_heuristics[i].strName);
    }
    
    return list;
}


int PredictorModel::toModel(unsigned int heuristic)
{
    for (int i = 0; i < _heuristics.size(); ++i)
    {
        if (_heuristics[i].heuristic == heuristic)
            return i;
    }
    
    return -1;
}


unsigned int PredictorModel::fromModel(int model_heuristic)
{
    // Assertions
    assert(model_heuristic >= 0);
    assert(model_heuristic < _heuristics.size());

    return _heuristics[model_heuristic].heuristic;
}


unsigned int PredictorModel::heuristicSeed(const std::string& strName)
{
    for (int i = 0; i < _heuristics.size(); ++i)
    {
        if ((_heuristics[i].strName == strName) || (_heuristics[i].strName == strName + "/1"))
            return _heuristics[i].seed;
    }
    
    return 0;
}

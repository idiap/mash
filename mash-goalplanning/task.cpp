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


/** @file   task.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Task' class
*/

#include "task.h"
#include <mash-utils/errors.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <dirent.h> 
#include <sys/stat.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

Task::Task()
: _bReadOnly(false)
{
}


Task::~Task()
{
}


/**************************** VIEWS-RELATED METHODS ***************************/

bool Task::reset()
{
    if (_bReadOnly)
        return false;

    tError ret = _perception.getController()->resetTask();
    if (ret != ERROR_NONE)
        return false;
    
    _perception._onStateReset();
    
    return true;
}


bool Task::performAction(unsigned int action, scalar_t* reward)
{
    assert(reward);
    
    if (_bReadOnly)
        return 0.0f;
    
    tError ret = _perception.getController()->performAction(action, reward);
    if (ret != ERROR_NONE)
        return false;
    
    _perception._onStateUpdated();
    
    if (!_strCaptureFolder.empty())
    {
        for (unsigned int view = 0; view < _perception.nbViews(); ++view)
        {
            Image* pImage = _perception._getView(view);
            if (pImage)
            {
                std::ostringstream strFileName;
                strFileName << std::setfill('0') << _strCaptureFolder << "sequence_" << std::setw(4) << _perception.currentSequence()
                            << "_view_" << std::setw(4) << view << "_frame_" << std::setw(4) << _perception.currentFrames() << ".ppm";
        
                std::ofstream stream;

                stream.open(strFileName.str().c_str(), std::ios::out | std::ios::binary);
                if (stream.is_open())
                {
                    std::ostringstream str;
                    str << "P6" << std::endl << pImage->width() << " " << pImage->height() << std::endl << 255 << std::endl;

                    stream.write(str.str().c_str(), (std::streamsize) str.str().length());
                    stream.write((char*) pImage->rgbBuffer(), pImage->width() * pImage->height() * 3 * sizeof(unsigned char));

                    stream.close();
                }
            }
        }
    }
        
    return reward;
}


void Task::setCaptureFolder(const std::string& strCaptureFolder)
{
    _strCaptureFolder = strCaptureFolder;

    if (!_strCaptureFolder.empty())
    {
        if (_strCaptureFolder.at(_strCaptureFolder.length() - 1) != '/')
            _strCaptureFolder += "/";
        
        // Create the folders if necessary
        size_t start = _strCaptureFolder.find("/", 0);
        while (start != string::npos)
        {
            string dirname = _strCaptureFolder.substr(0, start);
            
            DIR* d = opendir(dirname.c_str());
            if (!d)
                mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            else
                closedir(d);

            start = _strCaptureFolder.find("/", start + 1);
        }
    }
}

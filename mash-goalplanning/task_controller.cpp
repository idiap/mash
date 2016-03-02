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


/** @file   task_controller.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TaskController' class
*/

#include "task_controller.h"
#include <mash/imageutils.h>
#include <mash-utils/stringutils.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

TaskController::TaskController()
: _pClient(0), _bSupportRecordedSequences(false), _result(RESULT_NONE),
  _mode(GPMODE_STANDARD), _nbTrajectories(0), _suggestedAction(0)
{
}


TaskController::~TaskController()
{
}


/********************************* METHODS ************************************/

tError TaskController::setClient(Client* pClient)
{
    // Assertions
    assert(pClient);

    // Declarations
    string strResponse;
    ArgumentsList args;

    // Initialisations
    _pClient = pClient;
    
    // Sends an INFO request to the application server
    if (!_pClient->sendCommand("INFO", args))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Check that we are really connected to an application server
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "TYPE") || (args.size() != 1) || (args.getString(0) != "ApplicationServer"))
        return ERROR_SERVER_INCORRECT_TYPE;

    // Check that the application server serves images
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "SUBTYPE") || (args.size() != 1) || (args.getString(0) != "Interactive"))
        return ERROR_APPSERVER_INCORRECT_SUBTYPE;

    // Check that the application server uses the same protocol than us
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "PROTOCOL") || (args.size() != 1) ||
        ((args.getString(0) != "1.4") && (args.getString(0) != "1.3") && (args.getString(0) != "1.2")))
        return ERROR_APPSERVER_UNSUPPORTED_PROTOCOL;

    _bSupportRecordedSequences = (args.getString(0) == "1.4");

    // Sends an STATUS request to the application server
    if (!_pClient->sendCommand("STATUS", args))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Check that the application server isn't busy
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if (strResponse != "READY")
        return ERROR_SERVER_BUSY;
    
    return ERROR_NONE;
}


tError TaskController::selectTask(const std::string& strGoal, const std::string& strEnvironment,
                                  const tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pClient);
    assert(!strGoal.empty());
    assert(!strEnvironment.empty());

    // Declarations
    string strResponse;
    ArgumentsList args;
    tError ret;

    // Initialisations
    _result = RESULT_NONE;
    _mode = GPMODE_STANDARD;
    _nbTrajectories = 0;
    
    // Sends a INITIALIZE_TASK request to the application server
    args.add(strGoal);
    args.add(strEnvironment);
    if (!_pClient->sendCommand("INITIALIZE_TASK", args))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Retrieve the actions
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "AVAILABLE_ACTIONS") || (args.size() <= 1))
    {
        _strLastError = strResponse + " (expected: AVAILABLE_ACTIONS)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    for (unsigned int i = 0; i < args.size(); ++i)
        _actions.push_back(args.getString(i));

    // Retrieve the views
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "AVAILABLE_VIEWS") || (args.size() < 1))
    {
        _strLastError = strResponse + " (expected: AVAILABLE_VIEWS)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    for (unsigned int i = 0; i < args.size(); ++i)
    {
        tView view;
        
        tStringList parts = StringUtils::split(args.getString(i), ":", 1);
        if (parts.size() != 2)
        {
            _strLastError = strResponse + " (expected: AVAILABLE_VIEWS)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        view.strName = parts[0];

        parts = StringUtils::split(parts[1], "x", 1);
        if (parts.size() != 2)
        {
            _strLastError = strResponse + " (expected: AVAILABLE_VIEWS)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
        
        view.width = StringUtils::parseUnsignedInt(parts[0]);
        view.height = StringUtils::parseUnsignedInt(parts[1]);
        
        _views.push_back(view);
    }

    // Retrieve the mode
    if (_bSupportRecordedSequences)
    {
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if ((strResponse != "MODE") || (args.size() != 1))
        {
            _strLastError = strResponse + " (expected: MODE)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
        
        string strMode = args.getString(0);
        if (strMode == "TEACHER")
            _mode = GPMODE_TEACHER;
        else if (strMode == "RECORDED_TEACHER")
            _mode = GPMODE_RECORDED_TEACHER;
        else if (strMode == "RECORDED_TRAJECTORIES")
            _mode = GPMODE_RECORDED_TRAJECTORIES;
        else
            _mode = GPMODE_STANDARD;
    }

    // Sends the parameters to the application server
    if (!_pClient->sendCommand("BEGIN_TASK_SETUP", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;

    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if (strResponse != "OK")
    {
        _strLastError = strResponse + " (expected: OK)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    tExperimentParametersIterator iter, iterEnd;
    for (iter = parameters.begin(), iterEnd = parameters.end(); iter != iterEnd; ++iter)
    {
        if (!_pClient->sendCommand(iter->first, iter->second))
            return ERROR_NETWORK_REQUEST_FAILURE;

        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if (strResponse != "OK")
        {
            _strLastError = strResponse + " (expected: OK)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
    }

    if (!_pClient->sendCommand("END_TASK_SETUP", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;

    if (_bSupportRecordedSequences && (_mode != GPMODE_STANDARD))
    {
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if (strResponse != "SUGGESTED_ACTION")
        {
            _strLastError = strResponse + " (expected: SUGGESTED_ACTION)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
        
        string strAction = args.getString(0);
        
        for (_suggestedAction = 0; _suggestedAction < nbActions(); ++_suggestedAction)
        {
            if (_actions[_suggestedAction] == strAction)
                break;
        }
    }
    
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;

    if ((_bSupportRecordedSequences && (strResponse != "STATE_UPDATED")) ||
        (!_bSupportRecordedSequences && (strResponse != "OK")))
    {
        _strLastError = strResponse + " (expected: STATE_UPDATED)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    return ERROR_NONE;
}


tError TaskController::resetTask()
{
    // Assertions
    assert(_pClient);

    // Declarations
    string strResponse;
    ArgumentsList args;
    tError ret;
    
    // Sends a RESET_TASK request to the application server
    if (!_pClient->sendCommand("RESET_TASK", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;

    if (_bSupportRecordedSequences && (_mode != GPMODE_STANDARD))
    {
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if (strResponse != "SUGGESTED_ACTION")
        {
            _strLastError = strResponse + " (expected: SUGGESTED_ACTION)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
        
        string strAction = args.getString(0);
        
        for (_suggestedAction = 0; _suggestedAction < nbActions(); ++_suggestedAction)
        {
            if (_actions[_suggestedAction] == strAction)
                break;
        }
    }
    
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;

    if (strResponse != "STATE_UPDATED")
    {
        _strLastError = strResponse + " (expected: STATE_UPDATED)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    _result = RESULT_NONE;

    return ERROR_NONE;
}


unsigned int TaskController::nbTrajectories()
{
    // Declarations
    string strResponse;
    ArgumentsList args;

    // Use the cached value if available
    if (_nbTrajectories > 0)
        return _nbTrajectories;

    // Check that the mode provides trajectories
    if ((_mode != GPMODE_RECORDED_TEACHER) && (_mode != GPMODE_RECORDED_TRAJECTORIES))
        return 0;

    // Sends an GET_TRAJECTORIES_COUNT request to the application server
    if (!_pClient->sendCommand("GET_TRAJECTORIES_COUNT", ArgumentsList()))
        return 0;

    // Retrieve the number of trajectories
    if (!_pClient->waitResponse(&strResponse, &args))
        return 0;
    
    if ((strResponse != "NB_TRAJECTORIES") || (args.size() != 1))
        return 0;
    
    _nbTrajectories = (unsigned) args.getInt(0);

    return _nbTrajectories;
}


unsigned int TaskController::trajectoryLength(unsigned int trajectory)
{
    // Declarations
    string strResponse;
    ArgumentsList args;

    // Check that the mode provides trajectories
    if ((_mode != GPMODE_RECORDED_TEACHER) && (_mode != GPMODE_RECORDED_TEACHER))
        return 0;

    // Check that the trajectory is valid
    if (_nbTrajectories == 0)
        nbTrajectories();
    
    if (trajectory >= _nbTrajectories)
        return 0;

    // Sends an GET_TRAJECTORY_LENGTH request to the application server
    if (!_pClient->sendCommand("GET_TRAJECTORY_LENGTH", ArgumentsList((int) trajectory)))
        return 0;

    // Retrieve the length of the trajectory
    if (!_pClient->waitResponse(&strResponse, &args))
        return 0;
    
    if ((strResponse != "TRAJECTORY_LENGTH") || (args.size() != 1))
        return 0;
    
    return (unsigned) args.getInt(0);
}


Image* TaskController::getView(unsigned int index)
{
    // Assertions
    assert(_pClient);
    assert(index < nbViews());

    // Declarations
    string strResponse;
    ArgumentsList args;
    string strName;
    string strMimeType;
    int size;
    Image* pImage;

    // Sends an GET_VIEW request to the application server
    args.add(_views[index].strName);
    if (!_pClient->sendCommand("GET_VIEW", args))
        return 0;

    // Retrieve the infos about the image
    if (!_pClient->waitResponse(&strResponse, &args))
        return 0;
    
    if ((strResponse != "VIEW") || (args.size() != 3))
        return 0;

    strName = args.getString(0);
    strMimeType = args.getString(1);
    size = args.getInt(2);

    if ((strName != _views[index].strName) || (size <= 0))
        return 0;
    
    // Retrieve the bytes
    unsigned char* pBuffer = new unsigned char[size];
    if (!_pClient->waitData(pBuffer, size))
    {
        delete[] pBuffer;
        return 0;
    }
    
    // Create the image from the buffer
    pImage = ImageUtils::createImage(strMimeType, pBuffer, size);

    delete[] pBuffer;

    if (!pImage)
        return 0;

    // Add the missing pixel formats to the image
    ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_ALL);

    pImage->setView(index);

    return pImage;
}


unsigned int TaskController::suggestROIExtent()
{
    unsigned int size = 0;
    
    tViewsIterator iter, iterEnd;
    for (iter = _views.begin(), iterEnd = _views.end(); iter != iterEnd; ++iter)
    {
        if (size > 0)
            size = min(size, max(iter->width, iter->height));
        else
            size = max(iter->width, iter->height);
    }

    if ((size % 2) == 0)
        return (size - 2) >> 1;
    else
        return (size - 1) >> 1;
}


tError TaskController::performAction(unsigned int action, scalar_t* reward)
{
    // Assertions
    assert(_pClient);
    assert(action < nbActions());
    assert(reward);
    
    // Declarations
    string strResponse;
    ArgumentsList args;

    // Check that the action can be performed
    if (((_mode == GPMODE_RECORDED_TEACHER) || (_mode == GPMODE_RECORDED_TRAJECTORIES)) &&
        (action != _suggestedAction))
    {
        _strLastError = "Invalid action (using recorded trajectories)";
        return ERROR_APPSERVER_ERROR;
    }

    // Sends an ACTION request to the application server
    args.add(_actions[action]);
    if (!_pClient->sendCommand("ACTION", args))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Retrieve the reward
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if (strResponse != "REWARD")
    {
        _strLastError = strResponse + " (expected: REWARD)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    *reward = args.getFloat(0);

    while (true)
    {
        // Retrieve the event or the result
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;
    
        if (strResponse == "EVENT")
        {
        }
        else if (strResponse == "SUGGESTED_ACTION")
        {
            string strAction = args.getString(0);

            for (_suggestedAction = 0; _suggestedAction < nbActions(); ++_suggestedAction)
            {
                if (_actions[_suggestedAction] == strAction)
                    break;
            }
        }
        else if (strResponse == "STATE_UPDATED")
        {
            return ERROR_NONE;
        }
        else if (strResponse == "FINISHED")
        {
            _result = RESULT_GOAL_REACHED;
            return ERROR_NONE;
        }
        else if (strResponse == "FAILED")
        {
            _result = RESULT_TASK_FAILED;
            return ERROR_NONE;
        }
        else
        {
            _strLastError = strResponse + " (expected: EVENT | SUGGESTED_ACTION | STATE_UPDATED | FINISHED | FAILED)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }
    }
}

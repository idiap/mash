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


/** @file   image_database.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ImageDatabase' class
*/

#include "image_database.h"
#include <mash/imageutils.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

ImageDatabase::ImageDatabase(unsigned int maxNbImagesInCache)
: _pClient(0), _preferredRoiSize(0), _nbObjects(0), _cache(maxNbImagesInCache)
{
    _preferredImageSize.width = 0;
    _preferredImageSize.height = 0;
}


ImageDatabase::~ImageDatabase()
{
}


/********************************* METHODS ************************************/

tError ImageDatabase::setClient(Client* pClient)
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
    
    if ((strResponse != "SUBTYPE") || (args.size() != 1) || (args.getString(0) != "Images"))
        return ERROR_APPSERVER_INCORRECT_SUBTYPE;

    // Check that the application server uses the same protocol than us
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "PROTOCOL") || (args.size() != 1) || (args.getString(0) != "1.2"))
        return ERROR_APPSERVER_UNSUPPORTED_PROTOCOL;

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


tError ImageDatabase::setDatabase(const std::string& strName,
                                  const ArgumentsList& enabledLabels,
                                  bool bBackgroundImagesEnabled)
{
    // Assertions
    assert(_pClient);

    // Declarations
    string strResponse;
    ArgumentsList args;
    int nbImages;
    int nbLabels;
    tError ret;
    
    // Sends a SELECT_DATABASE request to the application server
    args.add(strName);
    if (!_pClient->sendCommand("SELECT_DATABASE", args))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Retrieve the infos from the database
    ret = receiveInfos(&nbImages, &nbLabels);
    if (ret != ERROR_NONE)
        return ret;

    // Sends a ENABLE_BACKGROUND_IMAGES or DISABLE_BACKGROUND_IMAGES request to
    // the application server
    if (!_pClient->sendCommand((bBackgroundImagesEnabled ? "ENABLE_BACKGROUND_IMAGES" : "DISABLE_BACKGROUND_IMAGES"), ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;

    // Retrieve the infos from the database
    ret = receiveInfos(&nbImages, &nbLabels);
    if (ret != ERROR_NONE)
        return ret;

    // Sends a ENABLE_LABELS request to the application server
    if (enabledLabels.size() > 0)
    {
        if (!_pClient->sendCommand("ENABLE_LABELS", enabledLabels))
            return ERROR_NETWORK_REQUEST_FAILURE;

        // Retrieve the infos from the database
        ret = receiveInfos(&nbImages, &nbLabels);
        if (ret != ERROR_NONE)
            return ret;
    }
    
    // Retrieves the list of label names
    if (!_pClient->sendCommand("LIST_LABEL_NAMES", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;
    
    while (true)
    {
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if (strResponse == "END_LIST_LABEL_NAMES")
            break;
    
        if ((strResponse != "LABEL_NAME") || (args.size() != 1))
        {
            _strLastError = strResponse + " (expected: LABEL_NAME)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        tLabel label;
        label.strName = args.getString(0);
        label.nbObjects = 0;

        _labels.push_back(label);
    }

    // Retrieves the preferred size of the images
    if (!_pClient->sendCommand("REPORT_PREFERRED_IMAGE_SIZE", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;
    
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;

    if ((strResponse == "PREFERRED_IMAGE_SIZE") && (args.size() == 2))
    {
        _preferredImageSize.width = args.getInt(0);
        _preferredImageSize.height = args.getInt(1);
    }

    // Retrieves the preferred size of the ROI
    if (!_pClient->sendCommand("REPORT_PREFERRED_ROI_SIZE", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;
    
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;

    if ((strResponse == "PREFERRED_ROI_SIZE") && (args.size() == 1))
        _preferredRoiSize = args.getInt(0);

    // Use the preprocessed images if possible
    if (_preferredImageSize.width > 0)
    {
        if (!_pClient->sendCommand("ENABLE_PREPROCESSED_IMAGES", ArgumentsList()))
            return ERROR_NETWORK_REQUEST_FAILURE;

        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;
    }

    // Retrieves the URL prefix for the images
    if (!_pClient->sendCommand("REPORT_URL_PREFIX", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;
    
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;

    if ((strResponse != "URL_PREFIX") || (args.size() != 1))
    {
        _strLastError = strResponse + " (expected: URL_PREFIX)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    _strImagesUrlPrefix = args.getString(0);
    if (_strImagesUrlPrefix.at(_strImagesUrlPrefix.length() - 1) != '/')
        _strImagesUrlPrefix += "/";

    // Retrieves the list of objects
    if (!_pClient->sendCommand("LIST_OBJECTS", ArgumentsList()))
        return ERROR_NETWORK_REQUEST_FAILURE;
    
    while (true)
    {
        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if (strResponse == "END_LIST_OBJECTS")
            break;

        if ((strResponse != "IMAGE") || (args.size() != 1))
        {
            _strLastError = strResponse + " (expected: IMAGE)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        int imageIndex = args.getInt(0);

        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if ((strResponse != "IMAGE_SIZE") || (args.size() != 2))
        {
            _strLastError = strResponse + " (expected: IMAGE_SIZE)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        tImage image;
        image.size.width = args.getInt(0);
        image.size.height = args.getInt(1);

        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if ((strResponse != "SET") || (args.size() != 1))
        {
            _strLastError = strResponse + " (expected: SET)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        if (args.getString(0) == "TRAINING")
            image.set = SET_TRAINING;
        else if (args.getString(0) == "TEST")
            image.set = SET_TEST;
        else
            image.set = SET_NONE;

        if (!_pClient->waitResponse(&strResponse, &args))
            return ERROR_NETWORK_RESPONSE_FAILURE;

        if ((strResponse != "NB_OBJECTS") || (args.size() != 1))
        {
            _strLastError = strResponse + " (expected: NB_OBJECTS)";
            return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
        }

        int nbObjectsInImage = args.getInt(0);

        for (int i = 0; i < nbObjectsInImage; ++i)
        {
            tObject object;
            
            if (!_pClient->waitResponse(&strResponse, &args))
                return ERROR_NETWORK_RESPONSE_FAILURE;

            if ((strResponse != "OBJECT_LABEL") || (args.size() != 1))
            {
                _strLastError = strResponse + " (expected: OBJECT_LABEL)";
                return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
            }

            object.label = args.getInt(0);
            
            _labels[object.label].nbObjects++;
            
            if (!_pClient->waitResponse(&strResponse, &args))
                return ERROR_NETWORK_RESPONSE_FAILURE;

            if ((strResponse != "OBJECT_COORDINATES") || (args.size() != 4))
            {
                _strLastError = strResponse + " (expected: OBJECT_COORDINATES)";
                return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
            }

            object.top_left.x = args.getInt(0);
            object.top_left.y = args.getInt(1);
            object.bottom_right.x = args.getInt(2);
            object.bottom_right.y = args.getInt(3);

            image.objects.push_back(object);
        }
        
        _images.push_back(image);
    }

    return ERROR_NONE;
}


Image* ImageDatabase::getImage(unsigned int index)
{
    // Assertions
    assert(index < nbImages());

    // Declarations
    string strUrl;
    Image* pImage;

    // Look in the cache first
    pImage = _cache.getImage(index);
    if (pImage)
        return pImage;

    // Retrieve the URL of the image
    strUrl = getImageUrl(index);

    // Download the image
    pImage = ImageUtils::loadImage(strUrl);
    if (!pImage)
        return 0;

    // Add the missing pixel formats to the image
    ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_ALL);

    // Add it to the cache
    _cache.addImage(index, pImage);
    
    return pImage;
}


std::string ImageDatabase::getImageUrl(unsigned int index)
{
    return _strImagesUrlPrefix + getImageName(index);
}


std::string ImageDatabase::getImageName(unsigned int index)
{
    // Assertions
    assert(_pClient);
    assert(index < nbImages());

    // Declarations
    string strResponse;
    ArgumentsList args;

    // Sends an IMAGE request to the application server
    args.add((int) index);
    if (!_pClient->sendCommand("IMAGE", args))
        return "";

    // Retrieve the URL of the image
    if (!_pClient->waitResponse(&strResponse, &args))
        return "";
    
    if ((strResponse != "IMAGE_NAME") || (args.size() != 1))
        return "";

    return args.getString(0);
}


tError ImageDatabase::receiveInfos(int* nbImages, int* nbLabels)
{
    // Assertions
    assert(_pClient);
    assert(nbImages);
    assert(nbLabels);

    // Declarations
    string strResponse;
    ArgumentsList args;
    
    // Retrieve the number of images in the database
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "NB_IMAGES") || (args.size() != 1))
    {
        _strLastError = strResponse + " (expected: NB_IMAGES)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    *nbImages = args.getInt(0);

    // Retrieve the number of labels in the database
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "NB_LABELS") || (args.size() != 1))
    {
        _strLastError = strResponse + " (expected: NB_LABELS)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    *nbLabels = args.getInt(0);

    // Retrieve the number of objects in the database
    if (!_pClient->waitResponse(&strResponse, &args))
        return ERROR_NETWORK_RESPONSE_FAILURE;
    
    if ((strResponse != "NB_OBJECTS") || (args.size() != 1))
    {
        _strLastError = strResponse + " (expected: NB_OBJECTS)";
        return ERROR_APPSERVER_UNEXPECTED_RESPONSE;
    }

    _nbObjects = args.getInt(0);

    return ERROR_NONE;
}

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


/** @file   declarations.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declarations used all over mash  
*/

#ifndef _MASHCLASSIFICATION_DECLARATIONS_H_
#define _MASHCLASSIFICATION_DECLARATIONS_H_

#include <mash-utils/declarations.h>
#include <mash/heuristic.h>
#include <vector>
#include <map>


//------------------------------------------------------------------------------
/// @brief  Contains all the types of MASH
//------------------------------------------------------------------------------
namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Contains some informations about an object in an image
    //--------------------------------------------------------------------------
    struct tObject
    {
        unsigned int    label;          ///< Label of the object
        bool            target;         ///< Indicates if the object has the
                                        ///< correct size
        coordinates_t   roi_position;   ///< Coordinates of the center of the
                                        ///< region of interest containing the
                                        ///< object
        unsigned int    roi_extent;     ///< Extent of the region of interest
                                        ///< (distance from the center to the
                                        ///< border)
    };

    //--------------------------------------------------------------------------
    /// @brief  Represents a list of the objects in an image
    ///
    /// How to iterate through the objects:
    /// <code>
    ///     tObjectsIterator iter, iterEnd;
    ///     for (iter = theList.begin(), iterEnd = theList.end(); iter != iterEnd; ++iter)
    ///     {
    ///         unsigned int label = iter->label;
    ///         coordinates_t top_left = iter->top_left;
    ///     }
    /// </code>
    //--------------------------------------------------------------------------
    typedef std::vector<tObject>            tObjectsList;
    typedef tObjectsList::const_iterator    tObjectsIterator;

    //----------------------------------------------------------------------
    /// @brief  Contains a list of coordinates
    //----------------------------------------------------------------------
    typedef std::vector<coordinates_t>  tCoordinatesList;
    typedef tCoordinatesList::iterator  tCoordinatesIterator;
}

#endif

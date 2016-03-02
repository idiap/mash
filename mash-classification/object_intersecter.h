/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Charles Dubout (charles.dubout@idiap.ch)
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


/** @file   intersecter.h
    @author Charles Dubout (charles.dubout@idiap.ch)

    Declaration of the 'tObjectIntersecter' class
*/

#ifndef _MASH_OBJECT_INTERSECTER_H_
#define _MASH_OBJECT_INTERSECTER_H_

#include "declarations.h"
#include <cmath>

namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Functor used to determine if two tObjects intersect (by
    ///         comparing the ratio of their intersection divided by their union
    ///         with a constant)
    //--------------------------------------------------------------------------
    class MASH_SYMBOL tObjectIntersecter
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param   objA    The first tObject to intersect
        /// @param   rho     The constant above which the objects intersect
        //----------------------------------------------------------------------
        tObjectIntersecter(const tObject& objA, scalar_t rho = 0.5f)
        : _objA(objA), _rho(rho) {}

        //_____ Public methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Determines if the two objects intersect
        //----------------------------------------------------------------------
        bool operator()(const tObject& objB) const
        {
            // Have to use int's as the extent might be larger than the position
            int x1 = std::max(int(_objA.roi_position.x) - int(_objA.roi_extent),
                              int(objB.roi_position.x) - int(objB.roi_extent));
            int x2 = int(std::min(_objA.roi_position.x + _objA.roi_extent,
                                  objB.roi_position.x + objB.roi_extent));

            if (x1 > x2)
                return false;

            int y1 = std::max(int(_objA.roi_position.y) - int(_objA.roi_extent),
                              int(objB.roi_position.y) - int(objB.roi_extent));
            int y2 = int(std::min(_objA.roi_position.y + _objA.roi_extent,
                                  objB.roi_position.y + objB.roi_extent));

            if (y1 > y2)
                return false;

            unsigned int inter = (unsigned int)(x2 - x1 + 1) * (unsigned int)(y2 - y1 + 1);
            unsigned int outer = (_objA.roi_extent * 2 + 1) * (_objA.roi_extent * 2 + 1) +
                                 (objB.roi_extent * 2 + 1) * (objB.roi_extent * 2 + 1) - inter;

            return inter >= _rho * outer; // Multiply both sides by outer
        }

        //_____ Attributes __________
    private:
        tObject     _objA;
        scalar_t    _rho;
    };

    // Adapter to use the tObjectIntersecter functor on bare coordinates
    class tCoordinatesIntersecter {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param   objA        The first tObject to intersect
        /// @param   roi_extent  The extent of the other objects
        /// @param   rho         The constant above which the objects intersect
        //----------------------------------------------------------------------
        tCoordinatesIntersecter(const tObject& objA, unsigned int roi_extent, scalar_t rho = 0.5f)
        : _intersecter(objA, rho), _roi_extent(roi_extent) {}

        //_____ Public methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Determines if the two objects intersect
        //----------------------------------------------------------------------
        bool operator()(coordinates_t roi_position) const
        {
            tObject object;
            object.roi_position = roi_position;
            object.roi_extent = _roi_extent;
            return _intersecter(object);
        }

        //_____ Attributes __________
    private:
        tObjectIntersecter  _intersecter;
        unsigned int        _roi_extent;
    };
}

#endif

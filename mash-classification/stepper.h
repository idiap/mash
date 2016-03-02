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


/** @file   stepper.h
    @author Charles Dubout (charles.dubout@idiap.ch)

    Declaration of the 'Stepper' class
*/

#ifndef _MASH_STEPPER_H_
#define _MASH_STEPPER_H_

#include "declarations.h"
#include <mash/heuristic.h>
#include <vector>

namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for the steppers
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Stepper
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Contains the list of the possible scales of an image
        //----------------------------------------------------------------------
        typedef std::vector<float>      tScalesList;
        typedef tScalesList::iterator   tScalesIterator;

        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param  detection   Object detection or classification (default
        ///                     classification)
        //----------------------------------------------------------------------
        Stepper(bool detection = false);

        //_____ Public methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the parameters of the stepper: roi extent and step
        ///            sizes
        //----------------------------------------------------------------------
        void setParameters(unsigned int roi_extent,
                           unsigned int step_x = 0,
                           unsigned int step_y = 0,
                           float        step_z = 0);

        //----------------------------------------------------------------------
        /// @brief  Indicates if the task is an object detection one
        //----------------------------------------------------------------------
        inline bool isDoingDetection() const
        {
            return _detection;
        }

        //----------------------------------------------------------------------
        /// @brief  Get the roi extent
        //----------------------------------------------------------------------
        inline unsigned int roiExtent() const
        {
            return _roi_extent;
        }

        //----------------------------------------------------------------------
        /// @brief  Get the size of a step in the X direction
        //----------------------------------------------------------------------
        inline unsigned int stepX() const
        {
            return _step_x;
        }

        //----------------------------------------------------------------------
        /// @brief  Get the size of a step in the Y direction
        //----------------------------------------------------------------------
        inline unsigned int stepY() const
        {
            return _step_y;
        }

        //----------------------------------------------------------------------
        /// @brief  Get the size of a step in the Z direction (scaling factor)
        //----------------------------------------------------------------------
        inline float stepZ() const
        {
            return _step_z;
        }

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with all the possible scales of
		///         the image
        ///
        /// @param  image_size  The dimensions of the image
        /// @retval list        The list of computed scales
        //----------------------------------------------------------------------
        void getScales(dim_t image_size,
                       tScalesList* list) const;

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the coordinates computed by
        ///         the stepper
        ///
        /// @param  image_size  The dimensions of the image
        /// @retval list        The list of coordinates computed
        //----------------------------------------------------------------------
        void getPositions(dim_t image_size,
                          tCoordinatesList* list) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the closest scanning position of the given
        ///         coordinates
        ///
        /// @param  image_size  The dimensions of the image
        /// @param  position    Position of the center of the ROI
        /// @return             The closest scanning position
        //----------------------------------------------------------------------
        coordinates_t getClosestPosition(dim_t image_size,
                                         coordinates_t position) const;

        //_____ Attributes __________
    private:
        bool         _detection;
        unsigned int _roi_extent;
        unsigned int _step_x;
        unsigned int _step_y;
        float        _step_z;
    };
}

#endif

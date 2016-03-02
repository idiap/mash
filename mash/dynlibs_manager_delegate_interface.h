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


/** @file   dynlibs_manager_delegate_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IDynlibsManagerDelegate' interface
*/

#ifndef _MASH_IDYNLIBSMANAGERDELEGATE_H_
#define _MASH_IDYNLIBSMANAGERDELEGATE_H_

#include <mash-utils/declarations.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface of the delegate object used by the 'dynamic libraries
    ///         manager' to interact with the sandbox
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IDynlibsManagerDelegate
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        IDynlibsManagerDelegate() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IDynlibsManagerDelegate() {}


        //_____ Methods to implement __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Enable the usage of the sandbox warden from now on
        //----------------------------------------------------------------------
        virtual void enableWarden() = 0;

        //----------------------------------------------------------------------
        /// @brief  Disable the usage of the sandbox warden from now on
        //----------------------------------------------------------------------
        virtual void disableWarden() = 0;
    };
}

#endif

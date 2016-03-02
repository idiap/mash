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


/** @file   dynlibs_manager_delegate.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'DynlibsManagerDelegate' class
*/

#ifndef _DYNLIBSMANAGERDELEGATE_H_
#define _DYNLIBSMANAGERDELEGATE_H_

#include <mash/dynlibs_manager_delegate_interface.h>
#include "warden.h"


//--------------------------------------------------------------------------
/// @brief  The delegate object used by the 'dynamic libraries manager' to
///         interact with the sandbox
//--------------------------------------------------------------------------
class DynlibsManagerDelegate: public Mash::IDynlibsManagerDelegate
{
    //_____ Construction / Destruction __________
public:
    //----------------------------------------------------------------------
    /// @brief  Constructor
    //----------------------------------------------------------------------
    DynlibsManagerDelegate(tWardenContext* pWardenContext)
    : _pWardenContext(pWardenContext)
    {
    }

    //----------------------------------------------------------------------
    /// @brief  Destructor
    //----------------------------------------------------------------------
    virtual ~DynlibsManagerDelegate() {}


    //_____ Methods to implement __________
public:
    //----------------------------------------------------------------------
    /// @brief  Enable the usage of the sandbox warden from now on
    //----------------------------------------------------------------------
    virtual void enableWarden()
    {
        setWardenContext(_pWardenContext);
    }

    //----------------------------------------------------------------------
    /// @brief  Disable the usage of the sandbox warden from now on
    //----------------------------------------------------------------------
    virtual void disableWarden()
    {
        setWardenContext(0);
    }


    //_____ Attributes __________
protected:
    tWardenContext* _pWardenContext;
};

#endif

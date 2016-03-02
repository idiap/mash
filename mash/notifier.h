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


/** @file   notifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Notifier' class
*/

#ifndef _MASH_NOTIFIER_H_
#define _MASH_NOTIFIER_H_

#include <mash/notifier_interface.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Concrete implementation of the INotifier interface, that
    ///         encapsulate another notifier
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Notifier: public INotifier
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Notifier();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Notifier();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Set the concrete notifier object to use to send the
        ///         notifications to the target
        //----------------------------------------------------------------------
        inline void useNotifier(INotifier* pNotifier)
        {
            _pNotifier = pNotifier;
        }


        //_____ Notifications __________
    public:
        //----------------------------------------------------------------------
        /// @copy   INotifier::onTrainingStepDone
        //----------------------------------------------------------------------
        virtual void onTrainingStepDone(unsigned int step,
                                        unsigned int nbTotalSteps = 0);

    
        //_____ Attributes __________
    protected:
        INotifier*  _pNotifier;     ///< The concrete notifier object to use to
                                    ///  send the notifications to the target
    };
}

#endif

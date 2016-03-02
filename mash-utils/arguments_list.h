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


/** @file   arguments_list.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ArgumentsList' class
*/

#ifndef _MASH_ARGUMENTSLIST_H_
#define _MASH_ARGUMENTSLIST_H_

#include "platform.h"
#include <sys/time.h>
#include <string>
#include <vector>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a list of arguments for a command
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ArgumentsList
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        ArgumentsList();

        //----------------------------------------------------------------------
        /// @brief  Creates a list with a string argument
        //----------------------------------------------------------------------
        ArgumentsList(const std::string& value);

        //----------------------------------------------------------------------
        /// @brief  Creates a list with an integer argument
        //----------------------------------------------------------------------
        ArgumentsList(int value);

        //----------------------------------------------------------------------
        /// @brief  Creates a list with a float argument
        //----------------------------------------------------------------------
        ArgumentsList(float value);

        //----------------------------------------------------------------------
        /// @brief  Creates a list with a timevalue argument
        //----------------------------------------------------------------------
        ArgumentsList(const struct timeval& value);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~ArgumentsList();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Append another arguments list at the end of this one
        //----------------------------------------------------------------------
        void append(const ArgumentsList& list);

        //----------------------------------------------------------------------
        /// @brief  Add a string argument to the list
        //----------------------------------------------------------------------
        void add(const std::string& value);

        //----------------------------------------------------------------------
        /// @brief  Add an integer argument to the list
        //----------------------------------------------------------------------
        void add(int value);

        //----------------------------------------------------------------------
        /// @brief  Add a float argument to the list
        //----------------------------------------------------------------------
        void add(float value);

        //----------------------------------------------------------------------
        /// @brief  Add a timevalue argument to the list
        //----------------------------------------------------------------------
        void add(const struct timeval& value);

        //----------------------------------------------------------------------
        /// @brief  Returns one of the argument as a string
        //----------------------------------------------------------------------
        std::string getString(unsigned int index) const;

        //----------------------------------------------------------------------
        /// @brief  Returns one of the argument as an integer
        //----------------------------------------------------------------------
        int getInt(unsigned int index) const;

        //----------------------------------------------------------------------
        /// @brief  Returns one of the argument as a float
        //----------------------------------------------------------------------
        float getFloat(unsigned int index) const;

        //----------------------------------------------------------------------
        /// @brief  Returns one of the argument as a timevalue
        //----------------------------------------------------------------------
        struct timeval getTimeval(unsigned int index) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of arguments in the list
        //----------------------------------------------------------------------
        inline int size() const
        {
            return _arguments.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Clear the arguments in the list
        //----------------------------------------------------------------------
        inline void clear()
        {
            _arguments.clear();
        }


        //_____ Internal types __________
    private:
        typedef std::vector<std::string>    tArgumentsVector;
        typedef tArgumentsVector::iterator  tArgumentsIterator;


        //_____ Attributes __________
    private:
        tArgumentsVector _arguments;
    };
}

#endif

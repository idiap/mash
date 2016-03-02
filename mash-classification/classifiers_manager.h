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


/** @file   classifiers_manager.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ClassifiersManager' class
*/

#ifndef _MASH_CLASSIFIERSMANAGER_H_
#define _MASH_CLASSIFIERSMANAGER_H_

#include "declarations.h"
#include "classifier.h"
#include <mash/dynlibs_manager.h>



namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Manage the loading/unloading of the classifiers
    ///
    /// Each classifier is contained in its own dynamic library. This class
    /// manages the loading of these libraries, and the creation of instances of
    /// their classifiers
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ClassifiersManager: public DynlibsManager
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param  strRootFolder   Path to the folder containing all the
        ///                         classifiers (in dynamic libraries form) 
        //----------------------------------------------------------------------
        ClassifiersManager(const std::string& strRootFolder = "");

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~ClassifiersManager();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns a new instance of a classifier
        ///
        /// @param  strName Name of the classifier in one of the following forms:
        ///                 'author/name/version', 'author/name' (if there is
        ///                 only one version) or 'name' (if the classifier isn't
        ///                 located in a user folder)
        /// @return         A new instance of the classifier
        ///
        /// @remark The first time the name is provided, the dynamic library
        ///         containing the classifier is loaded.
        //----------------------------------------------------------------------
        Classifier* create(const std::string& strName);


    private:
        tClassifierConstructor* getClassifierConstructor(const std::string& strClassifier);
    };
}

#endif

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


/** @file   sandboxed_classifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedClassifier' class
*/

#ifndef _SANDBOXEDCLASSIFIER_H_
#define _SANDBOXEDCLASSIFIER_H_

#include "sandboxed_object.h"
#include "sandbox_input_set.h"
#include "sandbox_notifier.h"
#include <mash-classification/classifiers_manager.h>
#include <mash-classification/classifier.h>


//------------------------------------------------------------------------------
/// @brief  Represents a sandboxed classifier
//------------------------------------------------------------------------------
class SandboxedClassifier: public ISandboxedObject
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    SandboxedClassifier(const Mash::CommunicationChannel& channel,
                        Mash::OutStream* pOutStream);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxedClassifier();


    //_____ Implementation of ISandboxedObject __________
public:
    virtual Mash::tError setPluginsFolder(const std::string& strPath);
    virtual Mash::tError loadPlugin(const std::string& strName);
    virtual Mash::tError createPlugins(Mash::OutStream* pOutStream,
                                       const std::vector<Mash::DataWriter>& dataWriters,
                                       const std::vector<Mash::DataWriter>& outCache,
                                       const std::vector<Mash::DataReader>& inCache,
                                       const Mash::PredictorModel& inModel,
                                       const Mash::DataReader& inInternalData,
                                       const Mash::PredictorModel& outModel,
                                       const Mash::DataWriter& outInternalData);
    virtual void handleCommand(Mash::tSandboxMessage command);


    //_____ Command handlers __________
private:
    Mash::tError handleSetSeedCommand();
    Mash::tError handleSetupCommand();
    Mash::tError handleLoadModelCommand();
    Mash::tError handleTrainCommand();
    Mash::tError handleClassifyCommand();
    Mash::tError handleReportFeaturesUsedCommand();
    Mash::tError handleSaveModelCommand();


    //_____ Internal types __________
protected:
    typedef Mash::tError (SandboxedClassifier::*tCommandHandler)();

    typedef std::map<Mash::tSandboxMessage, tCommandHandler>    tCommandHandlersList;
    typedef tCommandHandlersList::iterator                      tCommandHandlersIterator;

    
    //_____ Attributes __________
protected:
    static tCommandHandlersList handlers;

    Mash::ClassifiersManager*   _pManager;
    Mash::Classifier*           _pClassifier;
    std::string                 _strClassifierName;
    SandboxInputSet             _inputSet;
    SandboxNotifier             _notifier;
    tWardenContext              _wardenContext;
    Mash::PredictorModel        _inModel;
    Mash::DataReader            _inInternalData;
    Mash::PredictorModel        _outModel;
};

#endif

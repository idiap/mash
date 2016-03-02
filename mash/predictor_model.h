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


/** @file   predictor_model.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'PredictorModel' class
*/

#ifndef _MASH_PREDICTORMODEL_H_
#define _MASH_PREDICTORMODEL_H_

#include <mash-utils/declarations.h>
#include <mash-utils/data_writer.h>
#include <mash-utils/data_reader.h>
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Used to save and load the model used by a predictor
    ///
    /// The model keep an internal list of the heuristics it needs. They are
    /// indexed in model-space. Thus, when loading and saving a model, the
    /// heuristic indices must be converted from/to the model-space to the
    /// application-space.
    ///
    /// The format of the model file is predictor-dependent (text-based, binary,
    /// the content of the model, ...).
    ///
    /// Note that the model is saved in the data report, so a predictor-aware
    /// instrument might look at it.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL PredictorModel
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        PredictorModel();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~PredictorModel();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Create a file to contain the model
        ///
        /// @param  strFileName     Path to the file
        /// @return                 'true' if successful
        //----------------------------------------------------------------------
        bool create(const std::string& strFileName);

        //----------------------------------------------------------------------
        /// @brief  Open a file containing the model
        ///
        /// @param  strFileName     Path to the file
        /// @return                 'true' if successful
        //----------------------------------------------------------------------
        bool open(const std::string& strFileName);

        //----------------------------------------------------------------------
        /// @brief  Close the file
        //----------------------------------------------------------------------
        void close();

        //----------------------------------------------------------------------
        /// @brief  Delete the file
        //----------------------------------------------------------------------
        void deleteFile();
        
        //----------------------------------------------------------------------
        /// @brief  Returns the name of the file
        //----------------------------------------------------------------------
        std::string getFileName() const;

        //----------------------------------------------------------------------
        /// @brief  Indicates if the model is readable
        //----------------------------------------------------------------------
        inline bool isReadable() const
        {
            return _bReadable && _reader.isOpen();
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the model is writable
        //----------------------------------------------------------------------
        inline bool isWritable() const
        {
            return !_bReadable && _writer.isOpen();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the data writer to use to write data in the model
        //----------------------------------------------------------------------
        inline DataWriter& writer()
        {
            assert(!_bReadable);
            return _writer;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the data reader to use to read data from the model
        //----------------------------------------------------------------------
        inline DataReader& reader()
        {
            assert(_bReadable);
            return _reader;
        }


        //_____ Predictor management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the seed used by the predictor
        //----------------------------------------------------------------------
        void setPredictorSeed(unsigned int seed);

        //----------------------------------------------------------------------
        /// @brief  Returns the seed used by the predictor
        //----------------------------------------------------------------------
        inline unsigned int predictorSeed() const
        {
            return _predictorSeed;
        }


        //_____ Heuristics management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the (writable) model about an heuristic
        ///
        /// The model assign a model-space index to the heuristic.
        ///
        /// @remark Called by the Framework, the predictor can't use this method
        ///
        /// @remark The Framework only tell the model about the heuristics
        ///         reported as used by the predictor.
        //----------------------------------------------------------------------
        void addHeuristic(unsigned int heuristic, const std::string& strName,
                          unsigned int seed);

        //----------------------------------------------------------------------
        /// @brief  Tell the (readable) model about an heuristic
        ///
        /// The model retrieves the model-space index of the heuristic, if it
        /// was used to create the model.
        ///
        /// @remark Called by the Framework, the predictor can't use this method
        //----------------------------------------------------------------------
        void addHeuristic(unsigned int heuristic, const std::string& strName);

        //----------------------------------------------------------------------
        /// @brief  Tell the model that no more heuristic will be added to it
        ///
        /// @remark Called by the Framework, the predictor can't use this method
        //----------------------------------------------------------------------
        bool lockHeuristics();
        
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics used by the model
        //----------------------------------------------------------------------
        inline unsigned int nbHeuristics() const
        {
            return _heuristics.size();
        }
        
        //----------------------------------------------------------------------
        /// @brief  Returns a list of the names of the heuristics needed by the
        ///         model but not available in the current experiment
        ///
        /// @remark Only for readable models
        //----------------------------------------------------------------------
        tStringList missingHeuristics();
        
        //----------------------------------------------------------------------
        /// @brief  Convert an heuristic index to model-space
        //----------------------------------------------------------------------
        int toModel(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Convert an heuristic index from model-space
        //----------------------------------------------------------------------
        unsigned int fromModel(int model_heuristic);

        //----------------------------------------------------------------------
        /// @brief  Returns the seed to use for the specified heuristic
        //----------------------------------------------------------------------
        unsigned int heuristicSeed(const std::string& strName);


        //_____ Internal types __________
    private:
        struct tModelHeuristic
        {
            int             heuristic;
            std::string     strName;
            unsigned int    seed;
        };

        typedef std::vector<tModelHeuristic>    tModelHeuristics;
        typedef tModelHeuristics::iterator      tModelHeuristicIterator;


        //_____ Attributes __________
    private:
        tModelHeuristics    _heuristics;
        bool                _bLocked;
        bool                _bReadable;
        DataWriter          _writer;
        DataReader          _reader;
        unsigned int        _predictorSeed;
    };
}

#endif

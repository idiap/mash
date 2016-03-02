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


/** @file   classifier_input_set.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ClassifierInputSet' class
*/

#ifndef _MASH_CLASSIFIERINPUTSET_H_
#define _MASH_CLASSIFIERINPUTSET_H_

#include "declarations.h"
#include "classifier_input_set_interface.h"
#include "classifier_input_set_listener_interface.h"
#include "image_database.h"
#include "dataset.h"
#include <mash/features_computer.h>
#include <mash-utils/arguments_list.h>
#include <map>


namespace Mash
{
    // Forward declaration
    class Client;


    //--------------------------------------------------------------------------
    /// @brief  Concrete implementation of the Classifier Input Set
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ClassifierInputSet: public IClassifierInputSet
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param  maxNbSamplesInCaches    Maximum number of samples held in
        ///                                 the caches
        /// @param  detection               Indicates if the task is object
        ///                                 detection or classification
        //----------------------------------------------------------------------
        ClassifierInputSet(unsigned int maxNbSamplesInCaches,
                           bool detection);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~ClassifierInputSet();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the ID of the Inputs Set
        ///
        /// Each time something change in the Set (like the available images),
        /// the ID is changed too.
        //----------------------------------------------------------------------
        virtual unsigned int id()
        {
            return _id;
        }

        //----------------------------------------------------------------------
        /// @brief  Must be called when the data in the set has changed
        //----------------------------------------------------------------------
        inline void onUpdated()
        {
            ++_id;
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the task is an object detection one
        //----------------------------------------------------------------------
        virtual bool isDoingDetection() const
        {
            return _stepper.isDoingDetection();
        }

        //----------------------------------------------------------------------
        /// @brief  Set the client object to use
        ///
        /// @param  pClient     Client object to use to communicate with the
        ///                     application server. Must be already connected
        ///                     to an application server.
        /// @return             Error code
        ///
        /// @remark The set takes the ownership of the client object
        //----------------------------------------------------------------------
        tError setClient(Client* pClient);

        //----------------------------------------------------------------------
        /// @brief  Retrieves the client object used
        ///
        /// @return The client object used
        //----------------------------------------------------------------------
        inline Client* getClient()
        {
            return _database.getClient();
        }

        //----------------------------------------------------------------------
        /// @brief  Set the parameters of the experiment
        ///
        /// @param  parameters  List of the parameters
        /// @param  imagesSeed  Seed to use to select the images
        /// @return             Error code
        ///
        /// Available parameters:
        ///   - DATABASE_NAME <name>      (required)
        ///   - LABEL <your_label> <db_label1> <db_label2> ... ; <your_label>
        ///           <db_label1> <db_label2> ... (default: use all the labels
        ///     as defined by the database)
        ///   - TRAINING_SAMPLES <ratio>  (default: 0.5)
        ///   - ROI_SIZE <size> (default: 127)
        //----------------------------------------------------------------------
        tError setParameters(const tExperimentParametersList& parameters,
                             unsigned int imagesSeed);
 
        //----------------------------------------------------------------------
        /// @brief  Set the listener object to use to report events
        //----------------------------------------------------------------------
        inline void setListener(IClassifierInputSetListener* pListener)
        {
            _pListener = pListener;
        }

        //----------------------------------------------------------------------
        /// @brief  Must be called when the data in the set has changed
        //----------------------------------------------------------------------
        inline void restrictAccess(bool restricted)
        {
            _bRestrictedAccess = restricted;
        }

        //----------------------------------------------------------------------
        /// @brief  Enable/disable the computation of features
        //----------------------------------------------------------------------
        inline void setReadOnly(bool readOnly)
        {
            _bReadOnly = readOnly;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns a description of the last ERROR_EXPERIMENT_PARAMETER
        ///         error that occured
        //----------------------------------------------------------------------
        inline std::string getLastExperimentParameterError()
        {
            return (_strLastError.empty() ? _database.getLastError() : _strLastError);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured in the heuristics set
        //----------------------------------------------------------------------
        inline tError getLastHeuristicsError()
        {
            return _computer.getLastError();
        }


        //_____ Heuristics-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics()
        {
            return _computer.nbHeuristics();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic of
        ///         the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual unsigned int nbFeatures(unsigned int heuristic)
        {
            assert(_computer.initialized());
            return _computer.nbFeatures(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features of the set
        //----------------------------------------------------------------------
        virtual unsigned int nbFeaturesTotal()
        {
            assert(_computer.initialized());
            return _computer.nbFeaturesTotal();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The name of the heuristic, empty if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual std::string heuristicName(unsigned int heuristic)
        {
            return _computer.heuristicsSet()->heuristicName(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int heuristicSeed(unsigned int heuristic)
        {
            return _computer.heuristicSeed(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the given heuristic was referenced by the
        ///         classifier model that was loaded
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             'true' if the heuristic was in the model. When
        ///                     no model is used, 'false'.
        //----------------------------------------------------------------------
        virtual bool isHeuristicUsedByModel(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Mark a heuristic as used by the classifier model that was
        ///         loaded
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        void markHeuristicAsUsedByModel(unsigned int heuristic)
        {
            return _heuristicsInModel.push_back(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns a pointer to the Features Computer used
        //----------------------------------------------------------------------
        inline FeaturesComputer* featuresComputer()
        {
            return &_computer;
        }


        //_____ Images-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbImages()
        {
            return _dataset.nbImages();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of labels in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbLabels()
        {
            return _database.nbLabels();
        }

        //----------------------------------------------------------------------
        /// @brief  Computes several features of the specified heuristic on the
        ///         region of interest centered on a given point of an image
        ///
        /// @param  image       Index of the image
        /// @param  coordinates Coordinates of the center of the region of
        ///                     interest
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features to compute
        /// @param  indexes     Indexes of the features to compute
        /// @param  values[out] The computed features
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool computeSomeFeatures(unsigned int image,
                                         const coordinates_t& coordinates,
                                         unsigned int heuristic,
                                         unsigned int nbFeatures,
                                         unsigned int* indexes,
                                         scalar_t* values);

        //----------------------------------------------------------------------
        /// @brief  Returns the list of the objects in the specified image
        ///
        /// @param  image   Index of the image
        /// @retval objects The list of objects (with their labels) found in the
        ///                 image
        //----------------------------------------------------------------------
        virtual void objectsInImage(unsigned int image, tObjectsList* objects)
        {
            if (!_bRestrictedAccess)
                _dataset.objectsOfImage(image, objects);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the list of negatives (positions without any objects)
        ///         in the specified image
        ///
        /// @param  image           Index of the image
        /// @param  positions[out]  The positions of the computed negatives
        //----------------------------------------------------------------------
        virtual void negativesInImage(unsigned int image,
                                      tCoordinatesList* positions);

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified image
        ///
        /// @param  image   Index of the image
        ///
        /// @remark For images classification, all the images have the same size
        //----------------------------------------------------------------------
        virtual dim_t imageSize(unsigned int image)
        {
            return _dataset.imageSize(image);
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the image is part of the 'test set'
        ///
        /// @param  image   Index of the image
        ///
        /// @remark Only useful for the instruments, which can have access to
        ///         the full list of images
        //----------------------------------------------------------------------
        virtual bool isImageInTestSet(unsigned int image)
        {
            return (_dataset.getMode() == DataSet::MODE_TEST) ||
                   ((_dataset.getMode() == DataSet::MODE_NORMAL) && (_dataset.isImageInTestSet(image)));
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        virtual unsigned int roiExtent()
        {
            return _dataset.roiExtent();
        }
    
        //----------------------------------------------------------------------
        /// @brief  Returns the database object used
        //----------------------------------------------------------------------
        inline ImageDatabase* getDatabase()
        {
            return &_database;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the dataset object used
        //----------------------------------------------------------------------
        inline DataSet* getDataSet()
        {
            return &_dataset;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the stepper object used
        //----------------------------------------------------------------------
        inline Stepper* getStepper()
        {
            return &_stepper;
        }


        //_____ Attributes __________
    protected:
        unsigned int                    _id;
        FeaturesComputer                _computer;
        ImageDatabase                   _database;
        DataSet                         _dataset;
        Stepper                         _stepper;
        IClassifierInputSetListener*    _pListener;
        bool                            _bRestrictedAccess;
        std::string                     _strLastError;
        bool                            _bReadOnly;
        std::vector<unsigned int>       _heuristicsInModel;
    };
}

#endif

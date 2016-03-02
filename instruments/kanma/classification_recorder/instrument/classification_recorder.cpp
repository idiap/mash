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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    Records the results of the classification/object detection
*/

#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the instrument class
//------------------------------------------------------------------------------
class ClassificationRecorder: public Instrument
{
    //_____ Construction / Destruction __________
public:
    ClassificationRecorder();
    virtual ~ClassificationRecorder();


    //_____ Classifier-related events __________
public:
    //----------------------------------------------------------------------
    /// @brief  Called at the beginning of a classification or object
    ///         detection experiment (right after everything is initialized)
    ///
    /// @param  input_set   The Input Set provided to the classifier
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //----------------------------------------------------------------------
    virtual void onExperimentStarted(IClassifierInputSet* input_set);

    //--------------------------------------------------------------------------
    /// @brief  Called when the classifier finished to classify an object
    ///
    /// @param  input_set       The Input Set provided to the classifier
    /// @param  image           Index of the image in the current set
    ///                         (training or test)
    /// @param  original_image  Original index of the image (in the database)
    /// @param  position        Position of the ROI/object
    /// @param  results         Classification results returned by the classifier
    /// @param  error           Indicates if the classifier did an error, and
    ///                         which one
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                unsigned int image,
                                                unsigned int original_image,
                                                const coordinates_t& position,
                                                const Classifier::tClassificationResults& results,
                                                tClassificationError error);
};


//------------------------------------------------------------------------------
// Creation function of the instrument
//------------------------------------------------------------------------------
extern "C" Instrument* new_instrument()
{
    return new ClassificationRecorder();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

ClassificationRecorder::ClassificationRecorder()
{
}


ClassificationRecorder::~ClassificationRecorder()
{
}


/************************* CLASSIFIER-RELATED EVENTS **************************/

void ClassificationRecorder::onExperimentStarted(IClassifierInputSet* input_set)
{
    writer << "# FORMAT 1.1" << endl
           << "# ---------------" << endl
           << "# IMAGE image_index position.x position.y error expected_label" << endl
           << "# RESULT label score" << endl
           << "# RESULT label score" << endl
           << "# ..." << endl
           << "# ---------------" << endl
           << "# With:" << endl
           << "# error = - (None), FA (False Alarm), FR (False Rejection) or WC (Wrong Classification)" << endl
           << "# expected_label = - (None) or a label (only for classification tasks)" << endl;
}


void ClassificationRecorder::onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                            unsigned int image,
                                                            unsigned int original_image,
                                                            const coordinates_t& position,
                                                            const Classifier::tClassificationResults& results,
                                                            tClassificationError error)
{
    writer << "IMAGE " << original_image << " " << position.x << " " << position.y << " ";

    switch (error)
    {
        case CLASSIFICATION_ERROR_FALSE_ALARM:          writer << "FA"; break;
        case CLASSIFICATION_ERROR_FALSE_REJECTION:      writer << "FR"; break;
        case CLASSIFICATION_ERROR_WRONG_CLASSIFICATION: writer << "WC"; break;
        default:                                        writer << "-"; break;
    }

    writer << " ";

    // For classification tasks, search the true label of the object
    bool found = false;
    if (!input_set->isDoingDetection())
    {
        tObjectsList objects;
        input_set->objectsInImage(image, &objects);

        tObjectsList::iterator iter, iterEnd;
        for (iter = objects.begin(), iterEnd = objects.end(); iter != iterEnd; ++iter)
        {
            if ((iter->target) && (iter->roi_position.x - iter->roi_extent < position.x) &&
                (iter->roi_position.y - iter->roi_extent < position.y) &&
                (iter->roi_position.x + iter->roi_extent > position.x) &&
                (iter->roi_position.y + iter->roi_extent > position.y))
            {
                writer << iter->label;
                found = true;
                break;
            }
        }
    }

    if (!found)
        writer << "-";

    writer << endl;

    Classifier::tClassificationResults::const_iterator iter2, iterEnd2;
    for (iter2 = results.begin(), iterEnd2 = results.end(); iter2 != iterEnd2; ++iter2)
        writer << "RESULT " << iter2->first << " " << iter2->second << endl;
}

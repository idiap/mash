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

    Records the number of used features reported by the predictor
*/

#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the instrument class
//------------------------------------------------------------------------------
class FeatureList: public Instrument
{
    //_____ Construction / Destruction __________
public:
    FeatureList();
    virtual ~FeatureList();


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


    //_____ Goalplanner-related events __________
public:
    //----------------------------------------------------------------------
    /// @brief  Called when the learning phase of the goal-planner has
    ///         started (just before calling Planner::learn())
    ///
    /// @param  task    The task object provided to the goal-planner
    ///
    /// @remark Only available for goal-planning experiments
    //----------------------------------------------------------------------
    virtual void onPlannerLearningStarted(ITask* task);


    //_____ Predictor-related events __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Called when the list of features used by the predictor has
    ///         been retrieved
    ///
    /// @param  features    The list of features
    //--------------------------------------------------------------------------
    virtual void onFeatureListReported(const tFeatureList& features);


    //_____ Attributes __________
protected:
    std::vector<unsigned int> _nbFeaturesTotal;
};


//------------------------------------------------------------------------------
// Creation function of the instrument
//------------------------------------------------------------------------------
extern "C" Instrument* new_instrument()
{
    return new FeatureList();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

FeatureList::FeatureList()
{
}


FeatureList::~FeatureList()
{
}


/************************** CLASSIFIER-RELATED EVENTS *************************/

void FeatureList::onExperimentStarted(IClassifierInputSet* input_set)
{
    for (unsigned int i = 0; i < input_set->nbHeuristics(); ++i)
        _nbFeaturesTotal.push_back(input_set->nbFeatures(i));
}


/************************* GOALPLANNER-RELATED EVENTS *************************/

void FeatureList::onPlannerLearningStarted(ITask* task)
{
    for (unsigned int i = 0; i < task->perception()->nbHeuristics(); ++i)
        _nbFeaturesTotal.push_back(task->perception()->nbFeatures(i));
}


/************************** PREDICTOR-RELATED EVENTS **************************/

void FeatureList::onFeatureListReported(const tFeatureList& features)
{
    writer << "# FORMAT 1.0" << endl
           << "# ---------------" << endl
           << "# HEURISTIC heuristic_index total_features_count used_features_count feature_1 feature_2 ... feature_N" << endl
           << "# ..." << endl
           << "# ---------------" << endl;

    if (features.empty())
        return;

    typedef vector<unsigned int> tFeatureVector;

    vector<bool> filter(_nbFeaturesTotal.size(), true);
    
    tFeatureVector currentFeatures;
    unsigned int currentHeuristic = features.front().heuristic;
    
    tFeatureList::const_iterator iter, iterEnd;
    for (iter = features.begin(), iterEnd = features.end(); iter != iterEnd; ++iter)
    {
        if (currentHeuristic != iter->heuristic)
        {
            writer << "HEURISTIC " << currentHeuristic << " " << _nbFeaturesTotal[currentHeuristic] << " " << currentFeatures.size();

            tFeatureVector::iterator iter2, iterEnd2;
            for (iter2 = currentFeatures.begin(), iterEnd2 = currentFeatures.end(); iter2 != iterEnd2; ++iter2)
                writer << " " << *iter2;

            writer << endl;
            
            filter[currentHeuristic] = false;

            currentHeuristic = iter->heuristic;
            currentFeatures.clear();
        }
    
        currentFeatures.push_back(iter->feature_index);
    }

    writer << "HEURISTIC " << currentHeuristic << " " << _nbFeaturesTotal[currentHeuristic] << " " << currentFeatures.size();

    tFeatureVector::iterator iter2, iterEnd2;
    for (iter2 = currentFeatures.begin(), iterEnd2 = currentFeatures.end(); iter2 != iterEnd2; ++iter2)
        writer << " " << *iter2;

    writer << endl;

    filter[currentHeuristic] = false;

    for (unsigned int i = 0; i < _nbFeaturesTotal.size(); ++i)
    {
        if (filter[i])
            writer << "HEURISTIC " << i << " " << _nbFeaturesTotal[i] << " " << 0 << endl;
    }
}

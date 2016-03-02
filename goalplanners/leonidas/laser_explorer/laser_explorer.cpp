/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Leonidas Lefakis (leonidas.lefakis@idiap.ch)
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


/** Author: Leonidas Lefakis (leonidas.lefakis@idiap.ch)
 *
 * TODO: Write a description of your goal-planner
 */

#include <mash-classification/classifier.h>
#include <mash-goalplanning/planner.h>
#include <mash-goalplanning/task_interface.h>
#include <mash-goalplanning/perception.h>
#include <classification-utils/goal_planning_classifier_input_set.h>

// Directly import the classifier code
#include <francoisfleuret/simpleadaboost/simpleadaboost.cpp>
#include <francoisfleuret/simpleadaboost/fusion_sort.cpp>

#include <vector>
#include <map>

using namespace Mash;
using namespace std;
using namespace ClassificationUtils;


//------------------------------------------------------------------------------
// Declaration of the goal-planner class
//------------------------------------------------------------------------------
class laser: public Planner {

    typedef std::map<int, scalar_t> tClassificationResults;
    struct tDiffLabels{
        vector<unsigned int> labels;
        unsigned int nblabels;
    };

    //_____ Construction / Destruction __________
public:
    laser();
    virtual ~laser();


    //_____ Methods to implement __________
public:

    virtual bool setup(const tExperimentParametersList& parameters);
    virtual bool loadModel(PredictorModel &model);
    virtual bool learn(ITask* task);
    virtual unsigned int chooseAction(IPerception* perception);
    virtual bool reportFeaturesUsed(tFeatureList &list);
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:

    virtual void computeState(IPerception* perception);
    virtual unsigned int check_label(unsigned int sAction);

    vector<scalar_t> all_values;
    tDiffLabels diff_labels;

    Classifier *Laser_predictor;

    static const unsigned int   MEMORY_LIMIT;

    unsigned int max_nbFeatures;
    unsigned int nbLaserRounds;
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner() {
    return new laser();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

laser::laser() {
}


laser::~laser() {
    delete Laser_predictor;
}


/************************* IMPLEMENTATION OF Classifier ***********************/


bool laser::setup(const tExperimentParametersList& parameters) {
    Laser_predictor = new SimpleAdaboost();
    Laser_predictor->setup(parameters);

    nbLaserRounds = 100;
    max_nbFeatures = 0;

    for(tExperimentParametersIterator iter = parameters.begin();
    iter != parameters.end(); ++iter) {
        if(iter->first == "NB_ROUNDS") {
            nbLaserRounds = iter->second.getInt(0);
        }

        else if(iter->first == "MAX_NB_FEATURES") {
            max_nbFeatures = iter->second.getInt(0);
        }
    }
    return true;
}


bool laser::loadModel(PredictorModel &model) {
    return true;
}


bool laser::learn(ITask* task) {

    // Check that the task provides everything we need
    if ((task->capabilities() & IAS_CAP_INTERACTION) == 0)
    {
        outStream << "ERROR: The task isn't interactive" << endl;
        return false;
    }

    if ((task->capabilities() & IAS_CAP_SUGGESTED_ACTION) == 0)
    {
        outStream << "ERROR: The task doesn't provide suggested actions" << endl;
        return false;
    }

    const unsigned int max_nb_actions = 300;

    scalar_t reward[1];
    reward[0]=0.0f;

    coordinates_t coords;
    coords.x = 0;
    coords.y = 0;

    scalar_t train_error;

    GPClassifierInputSet* GPcis_train;
    if (max_nbFeatures==0) {
        GPcis_train = new GPClassifierInputSet(task->perception());
        max_nbFeatures = GPcis_train->maxNbFeatures();
    }
    else
        GPcis_train = new GPClassifierInputSet(task->perception(),max_nbFeatures);

    all_values.resize(GPcis_train->nbFeaturesTotal());

    bool follow_teacher = true;
    unsigned int random_action = 0;

    for (unsigned int LaserRound = 0; LaserRound<nbLaserRounds;LaserRound++)
    {
        unsigned int nbAct = 0;
        while ((task->result()==RESULT_NONE) && (nbAct<max_nb_actions))
        {
            if (GPcis_train->allocatedMemory()<MEMORY_LIMIT){
                computeState(task->perception());
                GPcis_train->pushData(all_values, task->perception()->viewSize(0), check_label(task->suggestedAction()));
            }

            float randomNumber = generator.randomize(1.0f);

            if (follow_teacher)
            {
                follow_teacher = (randomNumber > 0.05f);

                if (!follow_teacher)
                    random_action = generator.randomize(task->nbActions() - 1);
            }
            else
            {
                follow_teacher = (randomNumber < 0.1f);

                if (generator.randomize(1.0f) < 0.1f)
                    random_action = generator.randomize(task->nbActions() - 1);
            }

            if (follow_teacher)
            {
                if (!task->performAction(task->suggestedAction(), reward))
                    return false;
            }
            else
            {
                if (!task->performAction(random_action, reward))
                    return false;
            }

            nbAct++;
        }

        reward[0]=0.0f;
        task->reset();
        diff_labels.nblabels=diff_labels.labels.size();
        GPcis_train->setNumLabels(diff_labels.nblabels);
    }

    if (diff_labels.nblabels > 1) Laser_predictor->train(GPcis_train, train_error);

    delete GPcis_train;

    return true;
}


unsigned int laser::chooseAction(IPerception* perception) {
    coordinates_t coords;
    coords.x = 0;
    coords.y = 0;

    GPClassifierInputSet* GPcis_test;
    GPcis_test = new GPClassifierInputSet(perception,max_nbFeatures);
    GPcis_test->setNumLabels(diff_labels.nblabels);

    computeState(perception);
    GPcis_test->pushData(all_values, perception->viewSize(0), 0);

    tClassificationResults results;
    Laser_predictor->classify(GPcis_test, 0, coords, results);
    scalar_t cur_max = results[0];
    unsigned int ind_max = diff_labels.labels[0];
    for (unsigned int i = 1; i<diff_labels.nblabels;i++) {
        if (results[i]>cur_max) {
            cur_max = results[i];
            ind_max = diff_labels.labels[i];
        }
    }

    delete GPcis_test;
    return ind_max;
}


bool laser::reportFeaturesUsed(tFeatureList &list) {
    Laser_predictor->reportFeaturesUsed(list);
    return true;
}


bool laser::saveModel(PredictorModel &model) {
    return true;
}

void laser::computeState(IPerception* perception) {

    unsigned int all_k = 0;

    coordinates_t coords;
    coords.x = perception->roiExtent();
    coords.y = coords.x;
    for (unsigned int j=0; j< perception->nbHeuristics(); j++){
        unsigned int nbFeatures = min(perception->nbFeatures(j),max_nbFeatures);
        unsigned int indexes[nbFeatures];
        for (unsigned int k=0; k<nbFeatures; k++) indexes[k]=k;
        scalar_t values[nbFeatures];

        perception->computeSomeFeatures(0,
                coords,
                j,
                nbFeatures,
                indexes,
                values);

        for (unsigned int k=0; k<nbFeatures; k++) {
            all_values[k + all_k] = values[k];
        }

        all_k += nbFeatures;
    }
}

unsigned int laser::check_label(unsigned int sAction){

    for (unsigned int i = 0; i < diff_labels.labels.size();i++)
        if (diff_labels.labels[i] == sAction) return i;

    diff_labels.labels.push_back(sAction);
    return diff_labels.labels.size()-1;

}

const unsigned int laser::MEMORY_LIMIT = 1.5 * 1024 * 1024 * 1024;

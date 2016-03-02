#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class TaskTesterPlanner: public Planner
{
    //_____ Construction / Destruction __________
public:
    TaskTesterPlanner()
    {
    }

    virtual ~TaskTesterPlanner()
    {
    }


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        return true;
    }

    virtual bool loadModel(PredictorModel &model)
    {
        return true;
    }

    virtual bool learn(ITask* task)
    {
        tGoalPlanningMode mode = task->mode();
        
        unsigned int nbActions = task->nbActions();

        unsigned int nbTrajectories = task->nbTrajectories();
        for (unsigned int i = 0; i < nbTrajectories; ++i)
            task->trajectoryLength(i);
    
        scalar_t reward;
        for (unsigned int i = 0; i < nbActions; ++i)
        {
            task->performAction(i, &reward);
            task->suggestedAction();
        }
        
        task->reset();

        return true;
    }

    virtual unsigned int chooseAction(IPerception* perception)
    {
        return 0;
    }

    virtual bool reportFeaturesUsed(tFeatureList &list)
    {
        return true;
    }

    virtual bool saveModel(PredictorModel &model)
    {
        return true;
    }
};


extern "C" Planner* new_planner()
{
    return new TaskTesterPlanner();
}

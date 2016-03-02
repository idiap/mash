#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class NotifierTesterPlanner: public Planner
{
    //_____ Construction / Destruction __________
public:
    NotifierTesterPlanner()
    {
    }

    virtual ~NotifierTesterPlanner()
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
        notifier.onTrainingStepDone(5, 10);
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
    return new NotifierTesterPlanner();
}

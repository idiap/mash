#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class ModelSaverPlanner: public Planner
{
    //_____ Construction / Destruction __________
public:
    ModelSaverPlanner()
    : _nbHeuristics(0)
    {
    }

    virtual ~ModelSaverPlanner()
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
        _nbHeuristics = task->perception()->nbHeuristics();
        
        return true;
    }

    virtual unsigned int chooseAction(IPerception* perception)
    {
        return 0;
    }

    virtual bool reportFeaturesUsed(tFeatureList &list)
    {
        for (unsigned int i = 0; i < _nbHeuristics; ++i)
            list.push_back(tFeature(i, 0));

        return true;
    }

    virtual bool saveModel(PredictorModel &model)
    {
        if (!model.isWritable())
            return false;

        if (model.nbHeuristics() != _nbHeuristics)
            return false;

        model.writer() << "OK" << endl;

        return true;
    }


    // _____ Attributes __________
protected:
    unsigned int _nbHeuristics;
};


extern "C" Planner* new_planner()
{
    return new ModelSaverPlanner();
}

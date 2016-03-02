#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class ModelLoaderPlanner: public Planner
{
    //_____ Construction / Destruction __________
public:
    ModelLoaderPlanner()
    : _nbHeuristics(0)
    {
    }

    virtual ~ModelLoaderPlanner()
    {
    }


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        tExperimentParametersIterator iter;

        iter = parameters.find("NB_HEURISTICS");
        if (iter != parameters.end())
            _nbHeuristics = iter->second.getInt(0);

        return true;
    }

    virtual bool loadModel(PredictorModel &model)
    {
        if (!model.isReadable())
            return false;

        if (model.nbHeuristics() != _nbHeuristics)
            return false;

        const int64_t BUFFER_SIZE = 50;
        char buffer[BUFFER_SIZE];

        if (model.reader().tell() != 0)
            return false;

        if (model.reader().readline(buffer, BUFFER_SIZE) <= 0)
            return false;

        if (string(buffer) != "OK")
            return false;

        if (!model.reader().eof())
            return false;
                
        return true;
    }

    virtual bool learn(ITask* task)
    {
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


    // _____ Attributes __________
protected:
    unsigned int _nbHeuristics;
};


extern "C" Planner* new_planner()
{
    return new ModelLoaderPlanner();
}

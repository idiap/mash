#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class Report: public Planner
{
    //_____ Construction / Destruction __________
public:
    Report()
    {
    }

    virtual ~Report()
    {
    }


    //_____ Methods to implement __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        writer << "setup" << endl;
        
        return true;
    }

    virtual bool loadModel(PredictorModel &model)
    {
        writer << "loadModel" << endl;
        
        return true;
    }

    virtual bool learn(ITask* task)
    {
        writer << "learn" << endl;
        
        return true;
    }

    virtual unsigned int chooseAction(IPerception* perception)
    {
        writer << "chooseAction" << endl;

        return 0;
    }

    virtual bool reportFeaturesUsed(tFeatureList &list)
    {
        writer << "reportFeaturesUsed" << endl;

        return true;
    }

    virtual bool saveModel(PredictorModel &model)
    {
        writer << "saveModel" << endl;

        return true;
    }
};


extern "C" Planner* new_planner()
{
    return new Report();
}

#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class FewFeatures: public Planner
{
    //_____ Construction / Destruction __________
public:
    FewFeatures()
    {
    }

    virtual ~FewFeatures()
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
        return true;
    }

    virtual unsigned int chooseAction(IPerception* perception)
    {
        unsigned int roiExtent = perception->roiExtent();

        coordinates_t coords;
        coords.x = roiExtent;
        coords.y = roiExtent;

        unsigned int nbFeatures = min((unsigned int) 10, perception->nbFeatures(0));
                
        unsigned int* features = new unsigned int[nbFeatures];
        scalar_t* values = new scalar_t[nbFeatures];
        
        for (unsigned int k = 0; k < nbFeatures; ++k)
            features[k] = k;
                
        bool success = perception->computeSomeFeatures(0, coords, 0, nbFeatures, features, values);
    
        delete[] features;
        delete[] values;
    
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
    return new FewFeatures();
}

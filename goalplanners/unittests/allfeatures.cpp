#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


class AllFeatures: public Planner
{
    //_____ Construction / Destruction __________
public:
    AllFeatures()
    {
    }

    virtual ~AllFeatures()
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
        unsigned int nbViews = perception->nbViews();
        unsigned int nbHeuristics = perception->nbHeuristics();
        unsigned int roiExtent = perception->roiExtent();
        
        vector<unsigned int> nbFeatures;
        for (unsigned int i = 0; i < nbHeuristics; ++i)
        {
            nbFeatures.push_back(perception->nbFeatures(i));
        }
        
        coordinates_t coords;
        coords.x = roiExtent;
        coords.y = roiExtent;

        for (unsigned int i = 0; i < nbViews; ++i)
        {
            for (unsigned int j = 0; j < nbHeuristics; ++j)
            {
                unsigned int nbFeatures = perception->nbFeatures(i);
                
                unsigned int* features = new unsigned int[nbFeatures];
                scalar_t* values = new scalar_t[nbFeatures];

                for (unsigned int k = 0; k < nbFeatures; ++k)
                    features[k] = k;
                
                bool success = perception->computeSomeFeatures(i, coords, j, nbFeatures, features, values);
                
                delete[] features;
                delete[] values;
                
                if (!success)
                    return 0;
            }
        }

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
    return new AllFeatures();
}

#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


class TestInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    TestInstrument()
    {
    }

    virtual ~TestInstrument()
    {
    }
    

    //_____ Classifier-related events __________
public:
    virtual void onPlannerLearningStarted(ITask* task)
    {
        unsigned int nbViews = task->perception()->nbViews();
        unsigned int nbHeuristics = task->perception()->nbHeuristics();
        unsigned int roiExtent = task->perception()->roiExtent();
        
        vector<unsigned int> nbFeatures;
        for (unsigned int i = 0; i < nbHeuristics; ++i)
        {
            nbFeatures.push_back(task->perception()->nbFeatures(i));
            task->perception()->heuristicName(i);
        }
        
        coordinates_t coords;
        coords.x = roiExtent;
        coords.y = roiExtent;

        for (unsigned int i = 0; i < nbViews; ++i)
        {
            dim_t view_size = task->perception()->viewSize(i);
            
            for (unsigned int j = 0; j < nbHeuristics; ++j)
            {
                unsigned int* features = new unsigned int[nbFeatures[j]];
                scalar_t* values = new scalar_t[nbFeatures[j]];

                for (unsigned int k = 0; k < nbFeatures[j]; ++k)
                    features[k] = k;
                
                bool success = task->perception()->computeSomeFeatures(i, coords, j, nbFeatures[j], features, values);
                
                delete[] features;
                delete[] values;
                
                // Crash if we succeed at computing features
                if (success)
                {
                    int* p = 0; 
                    *p = 4;
                }
            }
        }
    }
};


extern "C" Instrument* new_instrument()
{
    return new TestInstrument();
}

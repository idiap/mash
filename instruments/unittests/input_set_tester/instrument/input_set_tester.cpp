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
    virtual void onClassifierTrainingStarted(IClassifierInputSet* input_set)
    {
        unsigned int id = input_set->id();
        unsigned int nbHeuristics = input_set->nbHeuristics();

        vector<unsigned int> nbFeatures;
        for (unsigned int i = 0; i < nbHeuristics; ++i)
        {
            nbFeatures.push_back(input_set->nbFeatures(i));
            input_set->heuristicName(i);
        }
        
        unsigned int nbImages = input_set->nbImages();
        unsigned int nbLabels = input_set->nbLabels();
        unsigned int roiExtent = input_set->roiExtent();

        for (unsigned int i = 0; i < nbImages; ++i)
        {
            dim_t size = input_set->imageSize(i);

            tObjectsList objects;
            input_set->objectsInImage(i, &objects);

            tCoordinatesList negatives;
            input_set->negativesInImage(i, &negatives);

            for (unsigned int j = 0; j < nbHeuristics; ++j)
            {
                unsigned int* features = new unsigned int[nbFeatures[j]];
                scalar_t* values = new scalar_t[nbFeatures[j]];

                for (unsigned int k = 0; k < nbFeatures[j]; ++k)
                    features[k] = k;

                coordinates_t coords;
                coords.x = 63;
                coords.y = 63;
                
                bool success = input_set->computeSomeFeatures(i, coords, j, nbFeatures[j], features, values);
                
                delete[] features;
                delete[] values;

                // // Crash if we succeeded at computing features
                // if (success)
                // {
                //     int* p = 0; 
                //     *p = 4;
                // }
            }
        }
    }
};


extern "C" Instrument* new_instrument()
{
    return new TestInstrument();
}

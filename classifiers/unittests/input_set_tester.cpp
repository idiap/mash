#include <mash-classification/classifier.h>
#include <vector>

using namespace Mash;
using namespace std;


class InputSetTesterClassifier: public Classifier
{
    //_____ Construction / Destruction __________
public:
    InputSetTesterClassifier()
    {
    }
    
    virtual ~InputSetTesterClassifier()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual bool setup(const tExperimentParametersList& parameters)
    {
        return true;
    }

    virtual bool loadModel(PredictorModel &model, DataReader &internal_data)
    {
        return true;
    }

    virtual bool train(IClassifierInputSet* input_set, scalar_t &train_error)
    {
        unsigned int id = input_set->id();
        unsigned int nbHeuristics = input_set->nbHeuristics();

        vector<unsigned int> nbFeatures;
        for (unsigned int i = 0; i < nbHeuristics; ++i)
        {
            nbFeatures.push_back(input_set->nbFeatures(i));
            input_set->heuristicName(i);
            input_set->heuristicSeed(i);
        }
        
        unsigned int nbImages = input_set->nbImages();
        unsigned int nbLabels = input_set->nbLabels();
        unsigned int roiExtent = input_set->roiExtent();

        for (unsigned int i = 0; i < nbImages; ++i)
        {
            dim_t size = input_set->imageSize(i);
            bool inTestSet = input_set->isImageInTestSet(i);

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
                
                if (!success)
                    return false;
            }
        }

        return true;
    }

    virtual bool classify(IClassifierInputSet* input_set,
                          unsigned int image,
                          const coordinates_t& position,
                          tClassificationResults &results)
    {
        return true;
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


extern "C" Classifier* new_classifier()
{
    return new InputSetTesterClassifier();
}

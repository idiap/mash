#ifndef _LINEAR_CLASSIFIER_H_
#define _LINEAR_CLASSIFIER_H_

#include <mash-classification/classifier_input_set_interface.h>
#include <mash-utils/outstream.h>
#include <mash/notifier.h>


class LinearClassifier
{
    //_____ Construction / Destruction __________
public:
    LinearClassifier();
    ~LinearClassifier();


    //_____ Methods __________
public:
    void setup(unsigned int label, const Mash::Notifier& notifier,
               const Mash::OutStream& outStream);

    bool train(Mash::IClassifierInputSet* input_set, unsigned int seed);

    bool adapt(Mash::IClassifierInputSet* input_set, unsigned int seed);

    bool classify(Mash::IClassifierInputSet* input_set, unsigned int image,
                  const Mash::coordinates_t& position, Mash::scalar_t &score);

    void reportFeaturesUsed(Mash::tFeatureList &list);

private:
    bool learn(Mash::IClassifierInputSet* input_set,
               unsigned int nbFeaturesTotal, unsigned int nbMaxSteps);
    

    //_____ Internal types __________
public:
    struct tHeuristicEntry
    {
        unsigned int    heuristic;
        unsigned int*   features;
        Mash::scalar_t* weights;
        unsigned int    nbFeatures;
    };


    //_____ Static attributes __________
public:
    static unsigned int nbMaxFeaturesPerHeuristic;
    static unsigned int nbMaxTrainingSteps;
    static unsigned int nbMaxAdaptationSteps;
    static unsigned int maxCacheSize;


    //_____ Attributes __________
public:
    unsigned int        _label;
    tHeuristicEntry*    _model;
    unsigned int        _modelSize;
    Mash::OutStream     _outStream;
    Mash::Notifier      _notifier;
};

#endif

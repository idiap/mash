#ifndef _MOCKPERCEPTION_H_
#define _MOCKPERCEPTION_H_

#include <mash-goalplanning/perception_interface.h>
#include <memory.h>


class MockPerception: public Mash::IPerception
{
    //_____ Construction / Destruction __________
public:
    MockPerception()
    {
        calls_counter_nbHeuristics          = 0;
        calls_counter_nbFeatures            = 0;
        calls_counter_heuristicName         = 0;
        calls_counter_heuristicSeed         = 0;
        calls_counter_nbViews               = 0;
        calls_counter_computeSomeFeatures   = 0;
        calls_counter_viewSize              = 0;
        calls_counter_roiExtent             = 0;
        nbComputedFeatures                  = 0;
    }
    
    virtual ~MockPerception()
    {
    }


    //_____ Heuristics-related methods __________
public:
    virtual unsigned int nbHeuristics()
    {
        ++calls_counter_nbHeuristics;

        return 2;
    }

    virtual unsigned int nbFeatures(unsigned int heuristic)
    {
        ++calls_counter_nbFeatures;

        return (heuristic == 0 ? 10 : 20000);
    }

    virtual unsigned int nbFeaturesTotal()
    {
        return 20010;
    }

    virtual std::string heuristicName(unsigned int heuristic)
    {
        ++calls_counter_heuristicName;

        char buffer[20];
        sprintf(buffer, "user1/mock%d", heuristic);

        return buffer;
    }

    virtual unsigned int heuristicSeed(unsigned int heuristic)
    {
        ++calls_counter_heuristicSeed;
        return 1000;
    }

    virtual bool isHeuristicUsedByModel(unsigned int heuristic)
    {
        return false;
    }


    //_____ Views-related methods __________
public:
    virtual unsigned int nbViews()
    {
        ++calls_counter_nbViews;

        return 1;
    }

    virtual bool computeSomeFeatures(unsigned int view,
                                     const Mash::coordinates_t& coordinates,
                                     unsigned int heuristic,
                                     unsigned int nbFeatures,
                                     unsigned int* indexes,
                                     Mash::scalar_t* values)
    {
        ++calls_counter_computeSomeFeatures;
        
        nbComputedFeatures += nbFeatures;

        memset(values, 0, nbFeatures * sizeof(Mash::scalar_t));

        return true;
    }

    virtual Mash::dim_t viewSize(unsigned int view)
    {
        ++calls_counter_viewSize;
        
        Mash::dim_t size;
        size.width = 320;
        size.height = 240;

        return size;
    }

    virtual unsigned int roiExtent()
    {
        ++calls_counter_roiExtent;

        return 49;
    }


    //_____ Attributes __________
public:
    unsigned int calls_counter_nbHeuristics;
    unsigned int calls_counter_nbFeatures;
    unsigned int calls_counter_heuristicName;
    unsigned int calls_counter_heuristicSeed;
    unsigned int calls_counter_nbViews;
    unsigned int calls_counter_computeSomeFeatures;
    unsigned int calls_counter_viewSize;
    unsigned int calls_counter_roiExtent;
    unsigned int nbComputedFeatures;
};

#endif

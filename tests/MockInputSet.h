#ifndef _MOCKINPUTSET_H_
#define _MOCKINPUTSET_H_

#include <mash-classification/classifier_input_set_interface.h>
#include <memory.h>
#include <stdio.h>


class MockInputSet: public Mash::IClassifierInputSet
{
    //_____ Construction / Destruction __________
public:
    MockInputSet()
    {
        calls_counter_nbHeuristics          = 0;
        calls_counter_nbFeatures            = 0;
        calls_counter_heuristicName         = 0;
        calls_counter_heuristicSeed         = 0;
        calls_counter_nbImages              = 0;
        calls_counter_nbLabels              = 0;
        calls_counter_computeSomeFeatures   = 0;
        calls_counter_objectsInImage        = 0;
        calls_counter_negativesInImage      = 0;
        calls_counter_imageSize             = 0;
        calls_counter_imageInTestSet        = 0;
        calls_counter_roiExtent             = 0;
        nbComputedFeatures                  = 0;
    }

    virtual ~MockInputSet()
    {
    }


    //_____ Methods __________
public:
    virtual unsigned int id()
    {
        return 0;
    }
    
    
    virtual bool isDoingDetection() const
    {
        return true;
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


    //_____ Images-related methods __________
public:
    virtual unsigned int nbImages()
    {
        ++calls_counter_nbImages;

        return 10;
    }

    virtual unsigned int nbLabels()
    {
        ++calls_counter_nbLabels;

        return 2;
    }

    virtual bool computeSomeFeatures(unsigned int image,
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

    virtual void objectsInImage(unsigned int image, Mash::tObjectsList* objects)
    {
        ++calls_counter_objectsInImage;

        Mash::tObject object;
        object.label          = 0;
        object.target         = true;
        object.roi_position.x = 63;
        object.roi_position.y = 63;
        object.roi_extent     = 63;

        objects->push_back(object);
    }

    virtual void negativesInImage(unsigned int image, Mash::tCoordinatesList* positions)
    {
        ++calls_counter_negativesInImage;

        Mash::coordinates_t position;
        position.x = 10;
        position.y = 10;

        positions->push_back(position);
    }

    virtual Mash::dim_t imageSize(unsigned int image)
    {
        ++calls_counter_imageSize;
        
        Mash::dim_t size;
        size.width = 127;
        size.height = 127;
        return size;
    }

    virtual bool isImageInTestSet(unsigned int image)
    {
        ++calls_counter_imageInTestSet;
        
        return false;
    }

    virtual unsigned int roiExtent()
    {
        ++calls_counter_roiExtent;
        
        return 63;
    }
    

public:
    unsigned int calls_counter_nbHeuristics;
    unsigned int calls_counter_nbFeatures;
    unsigned int calls_counter_heuristicName;
    unsigned int calls_counter_heuristicSeed;
    unsigned int calls_counter_nbImages;
    unsigned int calls_counter_nbLabels;
    unsigned int calls_counter_computeSomeFeatures;
    unsigned int calls_counter_objectsInImage;
    unsigned int calls_counter_negativesInImage;
    unsigned int calls_counter_imageSize;
    unsigned int calls_counter_imageInTestSet;
    unsigned int calls_counter_roiExtent;
    unsigned int nbComputedFeatures;
};

#endif

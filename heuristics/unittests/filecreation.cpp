#include <mash/heuristic.h>
#include <stdio.h>

using namespace Mash;


class FileCreationHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    FileCreationHeuristic() {}
    virtual ~FileCreationHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        FILE* pFile = fopen("PWND.txt", "w");
        if (pFile)
        {
            printf("PWND!\n");
            fclose(pFile);
 
            // Crash so the unit test can notice the failure
            int* pointer = 0;
            *pointer = 4;
        }
    }
    
    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new FileCreationHeuristic();
}

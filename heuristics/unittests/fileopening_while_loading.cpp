#include <mash/heuristic.h>
#include <stdio.h>
#include <string>

using namespace Mash;
using namespace std;


int doBadThings()
{
    FILE* pFile = fopen((string(MASH_DATA_DIR) + "car.ppm").c_str(), "rb");
    if (pFile)
    {
        printf("PWND!\n");
        fclose(pFile);

        // Crash so the unit test can notice the failure
        int* pointer = 0;
        *pointer = 4;
    }
}


static int A = doBadThings();


class FileOpeningHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    FileOpeningHeuristic() {}
    virtual ~FileOpeningHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }
    
    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new FileOpeningHeuristic();
}

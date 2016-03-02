#include <mash/heuristic.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace Mash;


class SocketHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    SocketHeuristic() {}
    virtual ~SocketHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        int	sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd != -1)
        {
            printf("PWND!\n");
            close(sd);
 
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
    return new SocketHeuristic();
}

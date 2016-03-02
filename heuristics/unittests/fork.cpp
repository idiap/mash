#include <mash/heuristic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace Mash;


class CommandExecutionHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    CommandExecutionHeuristic() {}
    virtual ~CommandExecutionHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        bool PWND = false;

        pid_t pid = fork();
        if (pid == 0)
        {
            _exit(0);
        }
        else if (pid > 0)
        {
            waitpid(pid, 0, 0);
            printf("PWND!\n");
            PWND = true;
        }

        if (PWND)
        {
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
    return new CommandExecutionHeuristic();
}

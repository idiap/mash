#include <mash/heuristic.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <mash-sandboxing/declarations.h>

using namespace Mash;
using namespace Mash::SandboxTimeBudgetDeclarations;


class TimeoutHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    TimeoutHeuristic()
    {
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);

        timeradd(&usage.ru_utime, &usage.ru_stime, &start);
    }

    virtual ~TimeoutHeuristic()
    {
    }


    //_____ Methods __________
public:
    void benchmark()
    {
        for (unsigned int i = 0; i < 20000000; ++i)
        {
            unsigned int a = i * i;
            unsigned int b = i * i;
            unsigned int c = i * i;
            unsigned int d = i * i;
            unsigned int e = i * i;
        }
    }
    
    inline void work(const struct timeval &maximum)
    { 
        while (true)
        {
            benchmark();

            struct timeval current, diff;
 
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);

            timeradd(&usage.ru_utime, &usage.ru_stime, &current);

            timersub(&current, &start, &diff);

            if (timercmp(&diff, &maximum, >=) != 0)
                break;
        }
    }

    
    inline void add(struct timeval &time, const struct timeval &additional, unsigned int multiplicator = 1)
    {
        long long n = additional.tv_usec * multiplicator;

        struct timeval tmp;
        struct timeval result;

        tmp.tv_usec = n % 1000000;
        tmp.tv_sec = additional.tv_sec * multiplicator + (n - tmp.tv_usec) / 1000000;

        timeradd(&time, &tmp, &result);
        
        time = result;
    }

    
    //_____ Attributes __________
public:
    struct timeval start;
};

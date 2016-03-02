#include "timeout_heuristic.h"

using namespace Mash;


class TestHeuristic: public TimeoutHeuristic
{
    //_____ Construction / Destruction __________
public:
    TestHeuristic()
    {
    }

    virtual ~TestHeuristic()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForCoordinates()
    {
        unsigned int roi_size = roi_extent * 2 + 1;
        
        struct timeval duration = { 1, 0 };

        add(duration, BUDGET_INITIALIZATION);
        add(duration, BUDGET_PER_SEQUENCE);
        add(duration, BUDGET_PER_PIXEL, image->width() * image->height());
        add(duration, BUDGET_PER_PIXEL, roi_size * roi_size);

        work(duration);
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new TestHeuristic();
}

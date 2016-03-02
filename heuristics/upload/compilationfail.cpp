#include <mash/heuristic.h>

using namespace Mash;


class CompilationFailHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    CompilationFailHeuristic() {}
    virtual ~CompilationFailHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForImage()
    {
        // Call an undeclared function
        undeclared_function();
    }
    
    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        // Use an undeclared variable
        return undeclared_variable;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new CompilationFailHeuristic();
}

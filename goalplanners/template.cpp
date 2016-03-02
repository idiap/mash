/** Author: YOUR_USERNAME

    TODO: Write a description of your goal-planner
*/

#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the goal-planner class
//
// TODO: Change the names of the class, the constructor and the destructor. Also
//       change the name of the class in the implementation of each method below
//------------------------------------------------------------------------------
class MyPlanner: public Planner
{
    //_____ Construction / Destruction __________
public:
    MyPlanner();
    virtual ~MyPlanner();


    //_____ Methods to implement __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Initialize the goal-planner
    ///
    /// @param  parameters  The goal-planner-specific parameters
    /// @return             'true' if successful
    //--------------------------------------------------------------------------
    virtual bool setup(const tExperimentParametersList& parameters);

    //----------------------------------------------------------------------
    /// @brief  Load a model
    ///
    /// @param  model   The model object to use
    /// @return         'true' if successful
    ///
    /// @note   This method will be called after setup(), but only if an
    ///         already trained model must be used. Then, either we begin
    ///         directly with calls to chooseAction(), or the goal-planner
    ///         is given the opportunity to adapt its model via a call to
    ///         learn().
    ///
    /// The provided model has already been initialized (ie. it already
    /// knows about the heuristics used by the goal-planner that saved it).
    //----------------------------------------------------------------------
    virtual bool loadModel(PredictorModel &model);

    //--------------------------------------------------------------------------
    /// @brief  Learn to solve the given task
    ///
    /// @param  task    The task to solve
    /// @return         'false' if an error occured
    //--------------------------------------------------------------------------
    virtual bool learn(ITask* task);

    //--------------------------------------------------------------------------
    /// @brief  Chooses an action from the given perception
    ///
    /// @param  perception  The perception of the task
    /// @return             The next action to perform
    ///
    /// This method will be used for evaluation.
    ///
    /// You can safely assume that the method will only be called after
    /// learn().
    //--------------------------------------------------------------------------
    virtual unsigned int chooseAction(IPerception* perception);

    //--------------------------------------------------------------------------
    /// @brief  Populates the provided list with the features used by the
    ///         goal-planner
    ///
    /// @retval list    The list of features used
    /// @return         'true' if successful
    ///
    /// You can safely assume that the method will only be called after
    /// learn(), and that the 'list' parameter is empty.
    //--------------------------------------------------------------------------
    virtual bool reportFeaturesUsed(tFeatureList &list);

    //----------------------------------------------------------------------
    /// @brief  Save the model trained by the goal-planner
    ///
    /// @param  model   The model object to use
    /// @return         'true' if successful
    ///
    /// @note   This method will only be called at the end of the experiment.
    ///
    /// The provided model has already been initialized (ie. it already
    /// knows about the heuristics used by the goal-planner, reported by
    /// reportFeaturesUsed()).
    //----------------------------------------------------------------------
    virtual bool saveModel(PredictorModel &model);


    //_____ Attributes __________
protected:
    // TODO: Declare all the attributes you'll need here
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//
// TODO: Change the name of the instanciated class
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
    return new MyPlanner();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

MyPlanner::MyPlanner()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


MyPlanner::~MyPlanner()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool MyPlanner::setup(const tExperimentParametersList& parameters)
{
    // TODO: Implement it
    return true;
}


bool MyPlanner::loadModel(PredictorModel &model)
{
    // TODO: Implement it
    return true;
}


bool MyPlanner::learn(ITask* task)
{
    // TODO: Implement it
    return true;
}


unsigned int MyPlanner::chooseAction(IPerception* perception)
{
    // TODO: Implement it
    return 0;
}


bool MyPlanner::reportFeaturesUsed(tFeatureList &list)
{
    // TODO: Implement it
    return true;
}


bool MyPlanner::saveModel(PredictorModel &model)
{
    // TODO: Implement it
    return true;
}

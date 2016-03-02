/** Author: YOUR_USERNAME

    TODO: Write a description of your instrument
*/

#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the instrument class
//
// TODO: Change the names of the class, the constructor and the destructor. Also
//       change the name of the class in the implementation of each method below
//------------------------------------------------------------------------------
class MyInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    MyInstrument();
    virtual ~MyInstrument();


    //_____ Methods __________
public:
    //----------------------------------------------------------------------
    /// @brief  Initialize the instrument
    ///
    /// @param  parameters  The instrument-specific parameters
    ///
    /// @note   The 'tExperimentParametersList' type is defined in
    ///         'mash-utils/declarations.h', with an usage example
    //----------------------------------------------------------------------
    virtual void setup(const tExperimentParametersList& parameters);


    //_____ General events __________
public:
    //----------------------------------------------------------------------
    /// @brief  Called at the end of an experiment
    //----------------------------------------------------------------------
    virtual void onExperimentDone();


    //_____ Classifier-related events __________
public:
    //----------------------------------------------------------------------
    /// @brief  Called at the beginning of a classification or object
    ///         detection experiment (right after everything is initialized)
    ///
    /// @param  input_set   The Input Set provided to the classifier
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //----------------------------------------------------------------------
    virtual void onExperimentStarted(IClassifierInputSet* input_set);

    //--------------------------------------------------------------------------
    /// @brief  Called when the training of the classifier has started (just
    ///         before calling Classifier::train())
    ///
    /// @param  input_set   The Input Set provided to the classifier
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierTrainingStarted(IClassifierInputSet* input_set);

    //--------------------------------------------------------------------------
    /// @brief  Called when the training of the classifier is done (after
    ///         Classifier::train() has returned, and the performance of the
    ///         classifier was tested on the training set)
    ///
    /// @param  input_set   The Input Set provided to the classifier
    /// @param  train_error The train error
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierTrainingDone(IClassifierInputSet* input_set,
                                          scalar_t train_error);

    //--------------------------------------------------------------------------
    /// @brief  Called when the test of the classifier has started
    ///
    /// @param  input_set   The Input Set provided to the classifier
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierTestStarted(IClassifierInputSet* input_set);

    //--------------------------------------------------------------------------
    /// @brief  Called when the test of the classifier is done
    ///
    /// @param  input_set   The Input Set provided to the classifier
    /// @param  test_error  The test error
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierTestDone(IClassifierInputSet* input_set,
                                      scalar_t test_error);

    //--------------------------------------------------------------------------
    /// @brief  Called when the classifier finished to classify an object
    ///
    /// @param  input_set       The Input Set provided to the classifier
    /// @param  image           Index of the image in the current set
    ///                         (training or test)
    /// @param  original_image  Original index of the image (in the database)
    /// @param  position        Position of the ROI/object
    /// @param  results         Classification results returned by the classifier
    /// @param  error           Indicates if the classifier did an error, and
    ///                         which one
    ///
    /// @remark Only available for classification and object detection
    ///         experiments
    //--------------------------------------------------------------------------
    virtual void onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                unsigned int image,
                                                unsigned int original_image,
                                                const coordinates_t& position,
                                                const Classifier::tClassificationResults& results,
                                                tClassificationError error);

    //--------------------------------------------------------------------------
    /// @brief  Called when some features have been computed by a classifier
    ///
    /// @param  detection       Indicates if the experiment is an
    ///                         object-detection one
    /// @param  training        Indicates if we are in the training or test
    ///                         phase
    /// @param  image           Index of the image in the current set
    ///                         (training or test)
    /// @param  original_image  Original index of the image (in the database)
    /// @param  coords          Position of the ROI
    /// #param  roiExtent       Extent of the ROI
    /// @param  heuristic       Index of the heuristic
    /// @param  nbFeatures      Number of features computed
    /// @param  indexes         Indexes of the computed features
    /// @param  values          Values of the features
    //--------------------------------------------------------------------------
    virtual void onFeaturesComputedByClassifier(bool detection,
                                                bool training,
                                                unsigned int image,
                                                unsigned int original_image,
                                                const coordinates_t& coords,
                                                unsigned int roiExtent,
                                                unsigned int heuristic,
                                                unsigned int nbFeatures,
                                                unsigned int* indexes,
                                                scalar_t* values);


    //_____ Goalplanner-related events __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Called when the learning phase of the goal-planner has
    ///         started (just before calling Planner::learn())
    ///
    /// @param  task    The task object provided to the goal-planner
    ///
    /// @remark Only available for goal-planning experiments
    //--------------------------------------------------------------------------
    virtual void onPlannerLearningStarted(ITask* task);

    //--------------------------------------------------------------------------
    /// @brief  Called when the learning phase of the goal-planner (after
    ///         Planner::learn() has returned)
    ///
    /// @param  task    The task object provided to the goal-planner
    /// @param  result  Result of the task
    ///
    /// @remark Only available for goal-planning experiments
    //--------------------------------------------------------------------------
    virtual void onPlannerLearningDone(ITask* task, tResult result);

    //--------------------------------------------------------------------------
    /// @brief  Called when the test of the goal-planner has started
    ///
    /// @param  task    The task object provided to the goal-planner
    ///
    /// @remark Only available for goal-planning experiments
    //--------------------------------------------------------------------------
    virtual void onPlannerTestStarted(ITask* task);

    //--------------------------------------------------------------------------
    /// @brief  Called when the test of the goal-planner is done
    ///
    /// @param  task    The task object provided to the goal-planner
    /// @param  score   Score
    /// @param  result  Result of the task
    ///
    /// @remark Only available for goal-planning experiments
    //--------------------------------------------------------------------------
    virtual void onPlannerTestDone(ITask* task, scalar_t score, tResult result);

    //--------------------------------------------------------------------------
    /// @brief  Called when the goal-planner has choosed the next action to
    ///         perform
    ///
    /// @param  task    The task object provided to the goal-planner
    /// @param  action  The action choosed by the goal-planner
    /// @param  reward  Reward received after performing the action
    /// @param  result  Result of the task
    ///
    /// @remark Only available for goal-planning experiments
    //--------------------------------------------------------------------------
    virtual void onPlannerActionChoosen(ITask* task, unsigned int action,
                                        scalar_t reward, tResult result);

    //--------------------------------------------------------------------------
    /// @brief  Called when some features have been computed by a
    ///         goal-planner
    ///
    /// @param  sequence    Index of the sequence
    /// @param  view        Index of the view
    /// @param  frame       Index of the frame
    /// @param  coords      Position of the ROI
    /// #param  roiExtent   Extent of the ROI
    /// @param  heuristic   Index of the heuristic
    /// @param  nbFeatures  Number of features computed
    /// @param  indexes     Indexes of the computed features
    /// @param  values      Values of the features
    //--------------------------------------------------------------------------
    virtual void onFeaturesComputedByPlanner(unsigned int sequence,
                                             unsigned int view,
                                             unsigned int frame,
                                             const coordinates_t& coords,
                                             unsigned int roiExtent,
                                             unsigned int heuristic,
                                             unsigned int nbFeatures,
                                             unsigned int* indexes,
                                             scalar_t* values);


    //_____ Predictor-related events __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Called when the list of features used by the predictor has
    ///         been retrieved
    ///
    /// @param  features    The list of features
    //--------------------------------------------------------------------------
    virtual void onFeatureListReported(const tFeatureList& features);


    //_____ Attributes __________
protected:
    // TODO: Declare all the attributes you'll need here
};


//------------------------------------------------------------------------------
// Creation function of the instrument
//
// TODO: Change the name of the instanciated class
//------------------------------------------------------------------------------
extern "C" Instrument* new_instrument()
{
    return new MyInstrument();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

MyInstrument::MyInstrument()
{
    // TODO: Initialization of the attributes that doesn't depend of anything
}


MyInstrument::~MyInstrument()
{
    // TODO: Cleanup of the allocated memory still remaining
}


/********************************** METHODS ***********************************/

void MyInstrument::setup(const tExperimentParametersList& parameters)
{
    // TODO: Implement it
}


/******************************* GENERAL EVENTS *******************************/

void MyInstrument::onExperimentDone()
{
    // TODO: Implement it
}


/************************* CLASSIFIER-RELATED EVENTS **************************/

void MyInstrument::onExperimentStarted(IClassifierInputSet* input_set)
{
    // TODO: Implement it
}


void MyInstrument::onClassifierTrainingStarted(IClassifierInputSet* input_set)
{
    // TODO: Implement it
}


void MyInstrument::onClassifierTrainingDone(IClassifierInputSet* input_set,
                                            scalar_t train_error)
{
    // TODO: Implement it
}


void MyInstrument::onClassifierTestStarted(IClassifierInputSet* input_set)
{
    // TODO: Implement it
}


void MyInstrument::onClassifierTestDone(IClassifierInputSet* input_set,
                                        scalar_t test_error)
{
    // TODO: Implement it
}


void MyInstrument::onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                  unsigned int image,
                                                  unsigned int original_image,
                                                  const coordinates_t& position,
                                                  const Classifier::tClassificationResults& results,
                                                  tClassificationError error)
{
    // TODO: Implement it
}


void MyInstrument::onFeaturesComputedByClassifier(bool detection,
                                                  bool training,
                                                  unsigned int image,
                                                  unsigned int original_image,
                                                  const coordinates_t& coords,
                                                  unsigned int roiExtent,
                                                  unsigned int heuristic,
                                                  unsigned int nbFeatures,
                                                  unsigned int* indexes,
                                                  scalar_t* values)
{
    // TODO: Implement it
}


/************************* GAOLPLANNER-RELATED EVENTS *************************/

void MyInstrument::onPlannerLearningStarted(ITask* task)
{
    // TODO: Implement it
}


void MyInstrument::onPlannerLearningDone(ITask* task, tResult result)
{
    // TODO: Implement it
}


void MyInstrument::onPlannerTestStarted(ITask* task)
{
    // TODO: Implement it
}


void MyInstrument::onPlannerTestDone(ITask* task, scalar_t score, tResult result)
{
    // TODO: Implement it
}


void MyInstrument::onPlannerActionChoosen(ITask* task, unsigned int action,
                                          scalar_t reward, tResult result)
{
    // TODO: Implement it
}


void MyInstrument::onFeaturesComputedByPlanner(unsigned int sequence,
                                               unsigned int view,
                                               unsigned int frame,
                                               const coordinates_t& coords,
                                               unsigned int roiExtent,
                                               unsigned int heuristic,
                                               unsigned int nbFeatures,
                                               unsigned int* indexes,
                                               scalar_t* values)
{
    // TODO: Implement it
}


/************************** PREDICTOR-RELATED EVENTS **************************/

void MyInstrument::onFeatureListReported(const tFeatureList& features)
{
    // TODO: Implement it
}

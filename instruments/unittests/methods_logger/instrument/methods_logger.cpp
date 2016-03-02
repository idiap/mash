#include <mash-instrumentation/instrument.h>

using namespace Mash;
using namespace std;


class TestInstrument: public Instrument
{
    //_____ Construction / Destruction __________
public:
    TestInstrument()
    {
    }

    virtual ~TestInstrument()
    {
    }
    

    virtual void setup(const tExperimentParametersList& parameters)
    {
        writer << "setup" << endl;
        
        tExperimentParametersIterator iter, iterEnd;
        for (iter = parameters.begin(), iterEnd = parameters.end(); iter != iterEnd; ++iter)
        {
            writer << "    " << iter->first;
            
            for (unsigned int i = 0; i < iter->second.size(); ++i)
                writer << " " << iter->second.getString(i) << endl;
        }
    }

    virtual void onExperimentDone()
    {
        writer << "onExperimentDone" << endl;
    }


    //_____ Classifier-related events __________
public:
    virtual void onExperimentStarted(IClassifierInputSet* input_set)
    {
        writer << "onExperimentStarted" << endl;
    }

    virtual void onClassifierTrainingStarted(IClassifierInputSet* input_set)
    {
        writer << "onClassifierTrainingStarted" << endl;
    }

    virtual void onClassifierTrainingDone(IClassifierInputSet* input_set,
                                          scalar_t train_error)
    {
        writer << "onClassifierTrainingDone" << endl;
    }

    virtual void onClassifierTestStarted(IClassifierInputSet* input_set)
    {
        writer << "onClassifierTestStarted" << endl;
    }

    virtual void onClassifierTestDone(IClassifierInputSet* input_set,
                                      scalar_t test_error)
    {
        writer << "onClassifierTestDone" << endl;
    }

    virtual void onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                unsigned int image,
                                                unsigned int original_image,
                                                const coordinates_t& position,
                                                const Classifier::tClassificationResults& results,
                                                tClassificationError error)
    {
        writer << "onClassifierClassificationDone" << endl;
    }

    virtual void onFeaturesComputedByClassifier(bool detection,
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
        writer << "onFeaturesComputedByClassifier" << endl;
    }


    //_____ Goalplanner-related events __________
public:
    virtual void onPlannerLearningStarted(ITask* task)
    {
        writer << "onPlannerLearningStarted" << endl;
    }

    virtual void onPlannerLearningDone(ITask* task, tResult result)
    {
        writer << "onPlannerLearningDone" << endl;
    }

    virtual void onPlannerTestStarted(ITask* task)
    {
        writer << "onPlannerTestStarted" << endl;
    }

    virtual void onPlannerTestDone(ITask* task, scalar_t score, tResult result)
    {
        writer << "onPlannerTestDone" << endl;
    }

    virtual void onPlannerActionChoosen(ITask* task, unsigned int action,
                                        scalar_t reward, tResult result)
    {
        writer << "onPlannerActionChoosen" << endl;
    }

    virtual void onFeaturesComputedByPlanner(unsigned int sequence,
                                             unsigned int view,
                                             unsigned int image,
                                             const coordinates_t& coords,
                                             unsigned int roiExtent,
                                             unsigned int heuristic,
                                             unsigned int nbFeatures,
                                             unsigned int* indexes,
                                             scalar_t* values)
    {
        writer << "onFeaturesComputedByPlanner" << endl;
    }


    //_____ Predictor-related events __________
public:
    virtual void onFeatureListReported(const tFeatureList& features)
    {
        writer << "onFeatureListReported" << endl;
    }
};


extern "C" Instrument* new_instrument()
{
    return new TestInstrument();
}

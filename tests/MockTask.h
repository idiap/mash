#ifndef _MOCKTASK_H_
#define _MOCKTASK_H_

#include <mash-goalplanning/task_interface.h>
#include "MockPerception.h"


class MockTask: public Mash::ITask
{
    //_____ Construction / Destruction __________
public:
    MockTask()
    {
        MODE                            = Mash::GPMODE_STANDARD;
        calls_counter_mode              = 0;
        calls_counter_nbActions         = 0;
        calls_counter_nbTrajectories    = 0;
        calls_counter_trajectoryLength  = 0;
        calls_counter_reset             = 0;
        calls_counter_performAction     = 0;
        calls_counter_suggestedAction   = 0;
    }
    
    virtual ~MockTask()
    {
    }


    //_____ Methods __________
public:
    virtual Mash::IPerception* perception()
    {
        return &mockPerception;
    }

    virtual Mash::tGoalPlanningMode mode()
    {
        ++calls_counter_mode;
        
        return MODE;
    }

    virtual unsigned int nbActions()
    {
        ++calls_counter_nbActions;
        
        return 4;
    }

    virtual unsigned int nbTrajectories()
    {
        ++calls_counter_nbTrajectories;
        
        return 5;
    }

    virtual unsigned int trajectoryLength(unsigned int trajectory)
    {
        ++calls_counter_trajectoryLength;
        
        return trajectory * 10;
    }

    virtual bool reset()
    {
        ++calls_counter_reset;
        
        return true;
    }

    virtual bool performAction(unsigned int action, Mash::scalar_t* reward)
    {
        ++calls_counter_performAction;
        
        return true;
    }

    virtual Mash::tResult result()
    {
        return Mash::RESULT_NONE;
    }

    virtual unsigned int suggestedAction()
    {
        ++calls_counter_suggestedAction;
        
        return 1;
    }


    //_____ Attributes __________
public:
    MockPerception          mockPerception;
    Mash::tGoalPlanningMode MODE;
    unsigned int            calls_counter_mode;
    unsigned int            calls_counter_nbActions;
    unsigned int            calls_counter_nbTrajectories;
    unsigned int            calls_counter_trajectoryLength;
    unsigned int            calls_counter_reset;
    unsigned int            calls_counter_performAction;
    unsigned int            calls_counter_suggestedAction;
};

#endif

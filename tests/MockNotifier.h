#ifndef _MOCKNOTIFIER_H_
#define _MOCKNOTIFIER_H_

#include <mash/notifier_interface.h>


class MockNotifier: public Mash::INotifier
{
    //_____ Construction / Destruction __________
public:
    MockNotifier()
    {
        step            = 0;
        nbTotalSteps    = 0;
    }
    
    virtual ~MockNotifier()
    {
    }


    //_____ Notifications __________
public:
    virtual void onTrainingStepDone(unsigned int step,
                                    unsigned int nbTotalSteps = 0)
    {
        this->step = step;
        this->nbTotalSteps = nbTotalSteps;
    }


    //_____ Attributes __________
public:
    unsigned int step;
    unsigned int nbTotalSteps;
};

#endif

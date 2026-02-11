#pragma once 

#include "csc.h"
#include "impulseFilter.h"
#include "actuators/pwmController.h"
#include "sensors/navigationSensors.h"
#include "sourceEvaluator.h"
#include <cstdint>
class SteeringOrchestrator {
    public: 
    
    SteeringOrchestrator(SourceEvaluator& eval, CoreSteeringController& csc, ImpulseFilter& filter, PWMController& pwm);

    void tick(NavigationSensors::NavigationSnapshot snapshot, uint32_t loopTimestamp);

    private: 

    SourceEvaluator & m_sourceEvaluator; 
    CoreSteeringController& m_csc;
    ImpulseFilter& m_impulsefilter; 
    PWMController& m_pwm;
    SteeringIntent m_steeringIntent;

};
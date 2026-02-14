#pragma once 

#include "core/steering/csc/csc.h"
#include "core/steering/impulseFilter.h"
#include "actuators/pwmController.h"
#include "sensors/navigationSensors.h"
#include "core/steering/sourceEvaluator.h"
#include "types/globalTypes.h"
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

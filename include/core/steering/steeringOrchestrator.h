#pragma once

#include "actuators/pwmController.h"
#include "core/steering/csc/csc.h"
#include "core/steering/impulseFilter.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

class SteeringOrchestrator {
public:
  SteeringOrchestrator(CoreSteeringController &csc, ImpulseFilter &filter,
                       PWMController &pwm);

  void tick(NavigationSensors::NavigationSnapshot snapshot,
            uint32_t loopTimestamp);

private:
  CoreSteeringController &m_csc;
  ImpulseFilter &m_impulsefilter;
  PWMController &m_pwm;
  SteeringIntent m_steeringIntent;
};

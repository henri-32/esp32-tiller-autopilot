#pragma once

#include "actuators/pwmController.h"
#include "core/steering/csc/csc.h"
#include "core/steering/impulseFilter.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

class SteeringOrchestrator {
public:
  // Contract:
  // Purpose: Connect CSC intent to actuator command execution.
  // Inputs: navigation snapshot + loop timestamp.
  // Outputs/Side-effects: issues PWM commands when intent exists.
  explicit SteeringOrchestrator(SteeringControllerConfig &config);

  void tick(NavigationSensors::NavigationSnapshot snapshot,
            uint16_t cscInternalTarget, uint32_t loopTimestamp);

private:
  ImpulseFilter m_impulsefilter;
  CoreSteeringController m_csc;
  PWMController m_pwm;
  SteeringIntent m_steeringIntent;
};

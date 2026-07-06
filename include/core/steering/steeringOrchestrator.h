#pragma once

#include "actuators/pwmController.h"
#include "core/steering/csc/csc.h"
#include "core/steering/impulseFilter.h"
#include "drivers/navigationSensors.h"
#include "types/steeringTypes.h"
#include <cstdint>

class SteeringOrchestrator {
public:
  // Contract:
  // Purpose: Connect CSC intent to actuator command execution.
  // Inputs: navigation snapshot + filtered CSC target + loop timestamp.
  // Outputs/Side-effects: issues PWM commands when intent exists.
  explicit SteeringOrchestrator(const SteeringControllerConfig &config);

  // TODO(Architektur):
  // Bei Einfuehrung eines GuidanceOutput diese Signatur auf einen expliziten
  // Input-Typ umstellen, statt Snapshot + getrenntem Target.
  void tick(const NavigationSensors::NavigationSnapshot &snapshot,
            uint16_t cscInternalTarget, uint32_t loopTimestamp);

private:
  ImpulseFilter m_impulsefilter;
  CoreSteeringController m_csc;
  PwmDriver m_pwm;
  SteeringIntent m_steeringIntent;
};

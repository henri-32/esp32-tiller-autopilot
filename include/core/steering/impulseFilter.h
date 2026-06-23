#pragma once
#include "core/config.h"
#include "drivers/navigationSensors.h"
#include "types/steeringTypes.h"

class ImpulseFilter {
public:
  // Contract:
  // Purpose: Shape an abstract steering intent into a filtered PWM intent.
  // Inputs: intent + navigation snapshot + mechanics/physics config slices.
  // Outputs/Side-effects: returns PWMIntent, no hardware side-effects.
  ImpulseFilter(const SteeringMechanicsConfig &mechanicsConfig,
                const SteeringPhysicsConfig &physicsConfig);

  PWMIntent apply(const SteeringIntent &intent,
                  const NavigationSensors::NavigationSnapshot &snapshot);

private:
  const SteeringMechanicsConfig &m_mechanics;
  const SteeringPhysicsConfig &m_physics;
};

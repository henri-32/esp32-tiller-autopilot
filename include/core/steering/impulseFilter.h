#pragma once
#include "core/config.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"

class ImpulseFilter {
public:
  // Contract:
  // Purpose: Shape an abstract steering intent into a PWM command.
  // Inputs: intent + navigation snapshot + mechanics/physics config slices.
  // Outputs/Side-effects: returns PWMCommand, no hardware side-effects.
  ImpulseFilter(SteeringMechanicsConfig &mechanicsConfig,
                SteeringPhysicsConfig &physicsConfig);

  PWMCommand apply(SteeringIntent intent,
                   NavigationSensors::NavigationSnapshot snapshot);

private:
  SteeringMechanicsConfig &m_mechanics;
  SteeringPhysicsConfig &m_physics;
};

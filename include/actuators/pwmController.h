#pragma once
#include "core/config.h"
#include "types/globalTypes.h"


class PWMController {
public:
  // Contract:
  // Purpose: Drive the steering actuator from a filtered PWM intent.
  // Inputs: PWMIntent with direction + filtered abstract impulse + duty cycle.
  // Outputs/Side-effects: updates actuator state (hardware output).
  enum class Force {
    Low,
    Medium,
    High
  }; // gerade noch Platzhalter, das wird der duty cycle.

  PWMController(const SteeringMechanicsConfig& config);
  PWMController(const PWMController &) = delete;
  PWMController &operator=(const PWMController &) = delete;

  void command(const PWMIntent &command);

private:
  const SteeringMechanicsConfig &m_config;
  SteeringDirection m_lastDir;
  Force m_lastFrc = Force::Low;
};

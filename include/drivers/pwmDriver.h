#pragma once
#include "config/config.h"
#include "types/steeringTypes.h"


class PwmDriver {
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

  PwmDriver(SteeringMechanicsConfig& conf): m_config(conf){};


  void command(const PWMIntent &command);

private:
  const SteeringMechanicsConfig &m_config;
  SteeringDirection m_lastDir;
  Force m_lastFrc = Force::Low;
};

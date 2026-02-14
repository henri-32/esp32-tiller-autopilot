#pragma once
#include "types/globalTypes.h"

class PWMController {
public:
  // Contract:
  // Purpose: Drive the steering actuator via PWM commands.
  // Inputs: PWMCommand with direction and pulse info.
  // Outputs/Side-effects: updates actuator state (hardware output).
  enum class Force { Low, Medium, High }; // gerade noch Platzhalter, das wird der duty cycle. 

  PWMController() = default;
  PWMController(const PWMController &) = delete;
  PWMController &operator=(const PWMController &) = delete;

  void command(PWMCommand command);

private:
  SteeringDirection m_lastDir;
  Force m_lastFrc = Force::Low;
};

#pragma once
#include "types/globalTypes.h"

class PWMController {
public:
  enum class Force { Low, Medium, High }; // gerade noch Platzhalter, das wird der duty cycle. 

  PWMController() = default;
  PWMController(const PWMController &) = delete;
  PWMController &operator=(const PWMController &) = delete;

  void command(PWMCommand command);

private:
  SteeringDirection m_lastDir;
  Force m_lastFrc = Force::Low;
};

#pragma once
#include "types/globalTypes.h"

class PWMController {
public:
  enum class Force { Low, Medium, High };

  PWMController() = default;
  PWMController(const PWMController &) = delete;
  PWMController &operator=(const PWMController &) = delete;

  void command(SteeringDirection dir);

private:
  SteeringDirection m_lastDir;
  Force m_lastFrc = Force::Low;
};

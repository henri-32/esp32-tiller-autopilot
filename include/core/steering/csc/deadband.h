#pragma once

#include "config/config.h"
#include <cstdint>

class Deadband {
public:
  // Contract:
  // Purpose: Reject small errors as noise.
  // Inputs: error + regulation config slice.
  // Outputs/Side-effects: boolean decision, no side-effects.
  Deadband(const SteeringRegulationConfig &config);

  bool errorSignificant(int16_t error);

private:
  const SteeringRegulationConfig &m_config;
};

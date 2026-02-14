#pragma once

#include "core/config.h"
#include <cstdint>

class Deadband {
public:
  // Contract:
  // Purpose: Reject small errors as noise.
  // Inputs: error + regulation config slice.
  // Outputs/Side-effects: boolean decision, no side-effects.
  Deadband(SteeringRegulationConfig &config);

  bool errorSignificant(int16_t error);

private:
  SteeringRegulationConfig &m_config;
};

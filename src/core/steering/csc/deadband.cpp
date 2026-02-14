#include "core/steering/csc/deadband.h"
#include <algorithm>

Deadband::Deadband(SteeringRegulationConfig &config) : m_config(config) {};

bool Deadband::errorSignificant(int16_t error) {

  // Treat small errors as noise.
  if (abs(error) <= m_config.steeringTolerance_deg) {
    return false;
  } else if (error > m_config.steeringTolerance_deg) {
    return true;
  };
  return false;
};

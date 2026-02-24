#include "core/steering/csc/deadband.h"
#include <cstdint>

Deadband::Deadband(const SteeringRegulationConfig &config) : m_config(config) {};

bool Deadband::errorSignificant(int16_t error) {
  if (error >  m_config.steeringTolerance_deg) return true;
  if (error < -m_config.steeringTolerance_deg) return true;
  return false;
}


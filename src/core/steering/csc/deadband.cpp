#include "core/steering/csc/deadband.h"

Deadband::Deadband(SteeringController_Config &config) : m_config(config) {};

bool Deadband::errorSignificant(int16_t error) {

  if (error <= m_config.regulations.steeringTolerance_deg) {
    return false;
  } else if (error > m_config.regulations.steeringTolerance_deg) {
    return true;
  };
  return false;
};
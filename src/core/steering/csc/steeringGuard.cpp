#include "core/steering/csc/steeringGuard.h"

SteeringGuard::SteeringGuard(SteeringRegulationConfig &config)
    : m_config(config) {};

bool SteeringGuard::observationBlocked(uint32_t loopTimestamp,
                                       uint32_t lastUpdate,
                                       uint32_t lastIntent) {
  // Minimum time between observations.
  if (loopTimestamp - lastUpdate < m_config.minimumTimeBtwObs_ms) {
    return true;
  };

  // Pause observations right after an impulse to wait for stabilization.
  if (loopTimestamp - lastIntent <
      m_config.pauseForValidObsAfterImpulse_ms) {
    return true;
  };

  return false;
};

bool SteeringGuard::intentBlocked(uint32_t loopTimestamp, uint32_t lastIntent,
                                  uint8_t sampleSize) {
  /*Semantisch eigentlich Hardwareschutz kommt vllt in den PWM später*/
  // Cooldown between actuator commands.
  if (loopTimestamp - lastIntent < m_config.steeringCooldown_ms) {
    return true;
  };

  // Require a minimum number of samples before commanding.
  if (sampleSize < m_config.minimumSampleSize) {
    return true;
  };
  return false;
};

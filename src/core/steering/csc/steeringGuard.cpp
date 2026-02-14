#include "core/steering/csc/steeringGuard.h"

SteeringGuard::SteeringGuard(SteeringController_Config &config)
    : m_config(config) {};

bool SteeringGuard::activeObservationGuard(uint32_t loopTimestamp,
                                           uint32_t lastUpdate,
                                           uint32_t lastIntent) {
  if (loopTimestamp - lastUpdate < m_config.regulations.minimumTimeBtwObs_ms) {
    return true;
  };

  if (loopTimestamp - lastIntent <
      m_config.regulations.pauseForValidObsAfterImpulse_ms) {
    return true;
  };

  return false;
};

bool SteeringGuard::activeIntentGuard(uint32_t loopTimestamp,
                                      uint32_t lastIntent, uint8_t sampleSize) {
  /*Semantisch eigentlich Hardwareschutz kommt vllt in den PWM später*/
  if (loopTimestamp - lastIntent < m_config.regulations.SteeringCooldown_ms) {
    return true;
  };

  if (sampleSize < m_config.regulations.minimumSampleSize) {
    return true;
  };
  return false;
};
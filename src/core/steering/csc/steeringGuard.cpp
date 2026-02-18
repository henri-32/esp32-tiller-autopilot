#include "core/steering/csc/steeringGuard.h"

SteeringGuard::SteeringGuard(SteeringRegulationConfig &config)
    : m_config(config), m_deadband(config) {};

bool SteeringGuard::observationBlocked(uint32_t loopTimestamp,
                                       uint32_t lastUpdate,
                                       uint32_t lastIntent) {
  // Minimum time between observations.
  if (loopTimestamp - lastUpdate < m_config.minimumTimeBtwObs_ms) {
    return true;
  };

  // Pause observations right after an impulse to wait for stabilization.
  if (loopTimestamp - lastIntent < m_config.pauseForValidObsAfterImpulse_ms) {
    return true;
  };

  return false;
};

bool SteeringGuard::intentBlocked(uint32_t loopTimestamp, uint32_t lastIntent,
                                  uint8_t sampleSize, int16_t median, float omega) {
  /*Semantisch eigentlich Hardwareschutz kommt vllt in den PWM später*/
  // Cooldown between actuator commands.
  if (loopTimestamp - lastIntent < m_config.steeringCooldown_ms) {
    return true;
  };

  // Require a minimum number of samples before commanding.
  if (sampleSize < m_config.minimumSampleSize) {
    return true;
  };

  // Der Median ist Action/NoAction Entscheidungsgrundlage
  // Daher ist er ein Intent guard. Errors werden unabhängig von der Toleranz im
  // Buffer gespeichert
  if (!m_deadband.errorSignificant(median)) {
    return true;
  };

  /* Wenn die Winkelgeschwindigkeit das gleiche Vorzeichen wie der Median hat 
  hat sie eine andere semantische Richtung (Fehler ist, was zum Target fehlt, nicht was vom Target weg ist)
  In dem Fall bewegt sich das System in die Richtige Richtung und es soll nicht zusätzlich gesteuert werden*/
  if (median < 0 && omega < 0 - m_config.omegaDeadband){
    return true;
  };
  if(median > 0 && omega > 0 + m_config.omegaDeadband){
    return true;
  };

  return false;
};

#pragma once
#include <cstdint>

struct SteeringMechanicsConfig {
  uint8_t dutyCycleDefault = 40;
  uint32_t steeringMinImpulse_ms = 100;
  uint32_t steeringMaxImpulse_ms = 3000;
};

struct SteeringRegulationConfig {
  // Regulation thresholds and timing gates used by CSC.
  uint8_t steeringTolerance_deg = 10;
  uint32_t steeringCooldown_ms = 2000;
  uint32_t minimumTimeBtwObs_ms = 1000;
  uint32_t pauseForValidObsAfterImpulse_ms = 3000;

  static constexpr uint8_t observationBufferSize = 70;
  uint8_t minimumSampleSize = 10;
};

struct SteeringSourceHandlingConfig {
  // Policy thresholds for selecting and validating sources.
  float minGpsSpeedForUse = 1.0;
  static constexpr uint8_t observationBufferSize = 70;
  uint32_t minTimeBetweenCorrection_ms = 20000;
};
struct SteeringPhysicsConfig {
  // Damping model for impulse shaping based on STW.
  float STWDampingRegulator = 0.4f;
  const float STWDampingMinFactor = 0.35f;
  const float STWDampingMaxFactor = 1.0f;
};
struct SteeringControllerConfig {
  // Aggregate config owned by the SystemController.
  SteeringMechanicsConfig mechanics;
  SteeringRegulationConfig regulations;
  SteeringPhysicsConfig physics;
  SteeringSourceHandlingConfig source;
};

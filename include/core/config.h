#pragma once
#include <cstdint>

struct SteeringMechanicsConfig {
  uint8_t dutyCycleDefault = 40;
  uint32_t SteeringMinImpulse_ms = 100;
  uint32_t SteeringMaxImpulse_ms = 3000;
};

struct SteeringRegulationConfig {
  uint8_t steeringTolerance_deg = 10;
  uint32_t SteeringCooldown_ms = 2000;
  uint32_t minimumTimeBtwObs_ms = 1000;
  uint32_t pauseForValidObsAfterImpulse_ms = 3000;

  static constexpr uint8_t observationBufferSize = 70;
  uint8_t minimumSampleSize = 10;
};

struct SteeringSourceEVConfig {
  float minimumGPS_SpeedForGPS_Use = 1.0;
  static constexpr uint8_t observationBufferSize = 70;
  uint32_t minimumTimeBeweenCorrection_ms = 20000;
};
struct SteeringPhysicsConfig {
  float stwDampingRegulator = 0.4f;
  const float stwDampingMinFactor = 0.35f;
  const float stwDampingMaxFactor = 1.0f;
};
struct SteeringController_Config {
  SteeringMechanicsConfig mechanics;
  SteeringRegulationConfig regulations;
  SteeringPhysicsConfig physics;
  SteeringSourceEVConfig source;
};

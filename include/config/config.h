#pragma once
#include <cstdint>

struct SteeringMechanicsConfig
{
  static constexpr uint8_t dutyCycleDefault = 40;
  static constexpr uint32_t steeringMinImpulse_ms = 100;
  static constexpr uint32_t steeringMaxImpulse_ms = 3000;
};

struct SteeringRegulationConfig
{
  // Regulation thresholds and timing gates used by CSC.
  static constexpr uint8_t steeringTolerance_deg = 1;
  static constexpr uint8_t counterNearTargetWindow_deg = 2;
  static constexpr int8_t smoothedMeanApplicationWindow_deg = 20;
  static constexpr uint32_t calculationWindowSmoothedMean = 4;
  static constexpr float omegaDeadband = 0.2;
  static constexpr uint8_t observationBufferSize = 70;
  // Runtime-active capacity used by CSC. Defaults to compile-time maximum.
  static constexpr uint8_t activeObservationBufferSize = observationBufferSize;
  static constexpr uint8_t minimumSampleSize = 10;
  static constexpr uint8_t omegaRobust = 3; // Describes Samplesize (errors) for Omega calculation
  static constexpr float omegaThresholdForCounter = 0.02;
  static constexpr uint8_t counterOmegaMinSampleSize = 3;
  static constexpr uint32_t minimumTimeBtwObs_ms = 1000;
  static constexpr uint32_t pauseForValidObsAfterImpulse_ms = 3000;
  static constexpr uint32_t steeringCooldown_ms = 2000;
  static constexpr uint32_t counterTimerGuard_ms = 2000;
  static constexpr uint32_t counterCooldown_ms = 7000;
};

struct SteeringSourceHandlingConfig
{
  // Policy thresholds for selecting and validating sources.
  static constexpr float minGpsSpeedForUse = 1.0;
  static constexpr uint32_t minTimeBetweenCorrection_ms = 20000;
  static constexpr uint8_t observationBufferSize = 70;
};
struct SteeringPhysicsConfig
{
  // Damping model for impulse shaping based on STW.
  static constexpr float STWDampingRegulator = 0.4f;
  static constexpr float STWDampingMinFactor = 0.35f;
  static constexpr float STWDampingMaxFactor = 1.0f;
};

struct DisplayConfig
{
  // DisplaySettings
  static constexpr uint32_t refreshRate_ms = 1000;
  static constexpr uint16_t displayWidth = 1080;
  static constexpr uint16_t displayHeight = 1440;
};

struct SteeringControllerConfig
{
  // Aggregate config owned by the SystemController.
  static constexpr SteeringMechanicsConfig mechanics{};
  static constexpr SteeringRegulationConfig regulations{};
  static constexpr SteeringPhysicsConfig physics{};
  static constexpr SteeringSourceHandlingConfig source{};
  static constexpr DisplayConfig display{};
};

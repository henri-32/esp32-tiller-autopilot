#pragma once
#include <cstdint>

enum class NavigationSource { Compass, Gps, Wind };
enum class SteeringDirection { Left, Right };

struct SteeringIntent {
  SteeringDirection dir;
  uint8_t abstractImpulse_0_100;
};

struct PWMCommand {
  SteeringDirection dir;
  uint32_t pulse_ms;
  uint8_t dutyCycle;
};

// Siehe README SteeringController_Config !!!
struct SteeringController_Config {

  // HDG steering mechanics
  uint8_t steeringTolerance_deg = 10;
  uint8_t abstractImpulseDefault = 100;
  uint8_t dutyCycleDefault = 40;
  uint32_t SteeringMinImpulse_ms = 100;
  uint32_t SteeringImpulseDefault_ms = 500;
  uint32_t SteeringMaxImpulse_ms = 1000;
  uint32_t SteeringCooldown_ms = 2000;
  const float stwDampingMinFactor = 0.35f;
  const float stwDampingMaxFactor = 1.0f;
  float stwDampingRegulator = 0.4f;

  uint32_t minimumTimeBtwObs_ms = 1000;
  static constexpr uint8_t observationBufferSize = 70;
  uint32_t pauseForValidObsAfterImpulse_ms = 3000;

  // Source Filters
  float minimumGPS_SpeedForGPS_Use = 1.0;
};

template <typename T> struct SensorSample {
  T value{};
  bool valid{false};
};
#pragma once
#include <Arduino.h>
enum class NavigationSource {Compass, Gps, Wind};
enum class SteeringDirection {Left, Right};
struct SystemConfig {
  uint16_t steeringTolerance_deg;
  uint16_t SteeringMinImpulse_ms;
  uint16_t SteeringMaxImpulse_ms;
  uint16_t SteeringCooldown_ms;
};

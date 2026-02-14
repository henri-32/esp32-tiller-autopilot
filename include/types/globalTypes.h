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

template <typename T> struct SensorSample {
  T value{};
  bool valid{false};
};


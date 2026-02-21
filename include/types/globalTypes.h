#pragma once
#include <cstdint>

enum class NavigationSource { Compass, Gps, Wind };

struct Intent {
  bool steeringEngaged = false;
  uint16_t generalTarget = 0;
  NavigationSource activeSource = NavigationSource::Compass;
};

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


struct SystemState {
  enum class SystemMode { INIT, OK, AISonly, SAFE };
  SystemMode systemMode = SystemMode::INIT;
};
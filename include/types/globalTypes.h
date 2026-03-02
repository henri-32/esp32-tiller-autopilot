#pragma once
#include <cstdint>

enum class NavigationSource { Compass, Gps, Wind };

struct SystemState {
  enum class SystemMode { INIT, OK, AISonly, SAFE };
  SystemMode systemMode = SystemMode::INIT;
};

struct Intent {
  bool steeringEngaged = false;
  uint16_t generalTarget = 0;
  NavigationSource activeSource = NavigationSource::Compass;
  SystemState requestedState;
};

enum class SteeringDirection { Left, Right };

struct SteeringIntent {
  SteeringDirection dir;
  uint8_t abstractImpulse_0_100;
};

struct PWMIntent {
  SteeringDirection dir;
  uint16_t filteredAbstractImpulse_0_100;
  uint8_t dutyCycle;
};

template <typename T> struct SensorSample {
  T value{};
  bool valid{false};
};


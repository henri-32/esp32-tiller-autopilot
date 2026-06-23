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


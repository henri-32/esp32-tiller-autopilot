#pragma once
#include "navigation/navigationTypes.h"
#include <cstdint>

#if 0
// Legacy synchronous SystemController/UI model. Kept as reference while the
// remaining ideas are migrated to the FreeRTOS-based architecture.
struct SystemState
{
  enum class SystemMode
  {
    INIT,
    OK,
    AISonly,
    SAFE
  };
  SystemMode systemMode = SystemMode::INIT;
};

struct Intent
{
  bool steeringEngaged = false;
  uint16_t generalTarget = 0;
  NavigationSource activeSource = NavigationSource::Compass;
  SystemState requestedState;
};
#endif

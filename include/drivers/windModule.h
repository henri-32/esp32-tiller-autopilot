#pragma once
#include "types/sensorTypes.h"
#include <cstdint>
#include <cstdlib>

class WindModule {
public:
  // Contract:
  // Purpose: Read wind angle.
  // Inputs: hardware sensor state.
  // Outputs/Side-effects: returns SensorSample; no side-effects.
  SensorSample<uint16_t> read() const;
};

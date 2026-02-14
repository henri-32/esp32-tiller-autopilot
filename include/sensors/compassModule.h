#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>

class CompassModule {
public:
  // Contract:
  // Purpose: Read compass heading.
  // Inputs: hardware sensor state.
  // Outputs/Side-effects: returns SensorSample; no side-effects.
  SensorSample<uint16_t> read() const;
};

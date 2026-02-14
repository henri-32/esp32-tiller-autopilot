#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>

class GPSModule {
public:
// Contract:
// Purpose: Read GPS course and speed over ground.
// Inputs: hardware sensor state.
// Outputs/Side-effects: returns SensorSample values; no side-effects.
SensorSample<uint16_t> readCOG() const;
SensorSample<float> readSOG() const;


};

#pragma once
#include "types/sensorTypes.h"
#include <cstdint>


class gpsDriver {
public:
// Contract:
// Purpose: Read GPS course and speed over ground.
// Inputs: hardware sensor state.
// Outputs/Side-effects: returns SensorSample values; no side-effects.
SensorSample<uint16_t> readCOG() const;
SensorSample<float> readSOG() const;


};

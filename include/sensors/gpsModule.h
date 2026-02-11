#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>

class GPSModule {
public:
SensorSample<uint16_t> readCOG() const;
SensorSample<float> readSOG() const;


};
#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>

class CompassModule {
public:
  SensorSample<uint16_t> read() const;
};

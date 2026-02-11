#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>

class WindModule {
public:
  SensorSample<uint16_t> read() const;
};

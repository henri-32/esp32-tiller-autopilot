#pragma once

#include <cstdint>

class GPSModel {
public:
  GPSModel();

  uint16_t applyFilter(uint16_t target);
};
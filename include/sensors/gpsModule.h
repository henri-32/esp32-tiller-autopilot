#pragma once
#include <cstdint>
#include <cstdlib>

class GPSModule {
public:
  uint16_t readHeading() const;
  float readSOG() const;
  bool isValue_valid() const;
};
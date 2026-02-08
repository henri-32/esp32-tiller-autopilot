#pragma once
#include <cstdint>

class WindModule {
public:
  uint16_t readHeading() const;
  bool isValue_valid() const;
};
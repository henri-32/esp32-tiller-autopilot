#pragma once
#include <cstdint>

class HeadingErrorCalculator {
public:
  HeadingErrorCalculator() = default;

  int16_t getCurrentError(uint16_t current, uint16_t target);
};
#pragma once

#include <cstdint>

class GPSModel {
public:
  // Contract:
  // Purpose: Apply GPS-specific filtering to a target course.
  // Inputs: target heading/course (uint16_t).
  // Outputs/Side-effects: filtered target; no side-effects.
  GPSModel();

  uint16_t applyFilter(uint16_t target);
};

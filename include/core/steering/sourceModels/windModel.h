#pragma once

#include <cstdint>

class WindModel{
public:
    // Contract:
    // Purpose: Apply wind-specific filtering to a target course.
    // Inputs: target heading/course (uint16_t).
    // Outputs/Side-effects: filtered target; no side-effects.
    WindModel();

    uint16_t applyFilter(uint16_t target);
};

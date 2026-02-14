#pragma once

#include <cstdint>

class WindModel{
public:
    WindModel();

    uint16_t applyFilter(uint16_t target);
};
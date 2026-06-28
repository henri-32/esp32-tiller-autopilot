#pragma once
#include "cstdint"

template <typename T> struct SensorSample {
  T value{};
  bool valid{false};
  uint32_t timestamp; 
};


#pragma once

template <typename T> struct SensorSample {
  T value{};
  bool valid{false};
};


#pragma once

#include <cstdint>

struct NmeaConfig
{
  static constexpr uint16_t max_sentence_len = 128;
  static constexpr uint8_t queue_depth = 10;
};

#pragma once
#include "config/nmeaConfig.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include <cstdint>

struct GpsConfig
{
  static constexpr uint16_t max_sentence_len = NmeaConfig::max_sentence_len;
};

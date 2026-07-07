#pragma once
#include "cstdint"


template <typename T> struct SensorSample
{
  T value{};
  bool valid{false};
  uint32_t timestamp;
};
struct Performance
{
  uint16_t free_task_stack = 0;
};


template <typename T> struct Telemetry
{
  T data{};
  Performance performance;
};

struct GpsData
{
  // lat and long in decimaldegree format DD.DDD...
  uint32_t latitude = 0;
  uint32_t longitude = 0;

  // SOG in knots
  float speed_kts = 0;

  // true COG
  uint16_t course_true = 0;

  // Quality of the satellite fix. 0 = invalid >0 are different types of valid (see nmea183
  // standard)
  uint8_t fixQuality = 0;

  // Number of satellites used for positioning
  uint8_t satellites_tracked = 0;
};


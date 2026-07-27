#pragma once
#include "cstdint"

template <typename T> struct SensorSample
{
  T value{};
  bool valid{false};
  uint64_t timestamp{0};
};
struct PerformanceData
{
  uint16_t free_task_stack = 0;
};

struct ErrorData
{
};

template <typename T> struct Telemetry
{
  T data{};
  PerformanceData performance;
  ErrorData error;
};

struct GpsData
{
  // lat and long in decimaldegree format DD.DDD...
  float latitude = 0.0F;
  float longitude = 0.0F;

  // SOG in knots
  float speed_kts = 0;

  // true COG
  uint16_t course_true = 0;

  // Quality of the satellite fix. 0 = invalid >0 are different types of valid (see nmea183
  // standard)
  uint8_t fixQuality = 0;

  // Number of satellites used for positioning
  uint8_t satellites_tracked = 0;

  // esp_get_time() from parsed VTG Sentence
  uint64_t timestamp = 0;
};


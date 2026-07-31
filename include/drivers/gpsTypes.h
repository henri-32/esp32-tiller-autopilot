#pragma once

#include <cstdint>

struct gpsDriverData_t
{
  // Latitude and longitude in decimal degree format DD.DDD...
  float latitude = 0.0F;
  float longitude = 0.0F;

  // Speed over ground in knots.
  float speed_kts = 0.0F;

  // True course over ground in degrees.
  uint16_t course_true = 0;

  // 0 is invalid; positive values represent valid NMEA fix qualities.
  uint8_t fixQuality = 0;
  uint8_t satellites_tracked = 0;

  // esp_timer_get_time() captured when the VTG sentence was parsed.
  uint64_t timestamp = 0;
};

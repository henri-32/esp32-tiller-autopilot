#pragma once

#include <cstdint>

template <typename T> struct SensorSample
{
  T value{};
  bool valid{false};
  uint64_t timestamp{0};
};

enum class NavigationSource : uint8_t
{
  Compass,
  Gps,
  Wind
};

struct NavigationSnapshot_t
{
  SensorSample<uint16_t> compass_hdg_dg{};
  SensorSample<uint16_t> gps_cog_dg{};
  SensorSample<float> gps_sog_kts{};
  uint16_t target_course{};
  float gps_lat = 0.0F;
  float gps_lon = 0.0F;
  SensorSample<uint16_t> wind_angle_dg{};
  SensorSample<float> stw_kts{};
  NavigationSource lead_source = NavigationSource::Compass;
  bool steering_engaged = false;
};

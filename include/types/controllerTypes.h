#pragma once
#include <cstdint> 
#include "types/sensorTypes.h"

enum class NavigationSource { Compass, Gps, Wind };

struct SystemState {
  enum class SystemMode { INIT, OK, AISonly, SAFE };
  SystemMode systemMode = SystemMode::INIT;
};

struct Intent {
  bool steeringEngaged = false;
  uint16_t generalTarget = 0;
  NavigationSource activeSource = NavigationSource::Compass;
  SystemState requestedState;
};


// Navigation values and their validity/timestamp metadata.
struct NavigationSnapshot
//{{{
{
  SensorSample<uint16_t> compass_hdg_dg;
  SensorSample<uint16_t> gps_cog_dg;
  SensorSample<float> gps_sog_kts;
  SensorSample<uint16_t> wind_angle_dg;
  SensorSample<float> stw_kts;
  NavigationSource LeadSource;
};
//}}}


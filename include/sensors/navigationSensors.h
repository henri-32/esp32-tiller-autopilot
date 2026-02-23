#pragma once
#include "sensors/NMEA183BUS.h"
#include "sensors/compassModule.h"
#include "sensors/gpsModule.h"
#include "sensors/windModule.h"
#include "types/globalTypes.h"
#include <cstdint>

class NavigationSensors {
public:
  // Contract:
  // Purpose: Read and aggregate navigation sensor data into a snapshot.
  // Inputs: hardware modules + lead source selection.
  // Outputs/Side-effects: returns snapshot; updates lead source state.
  explicit NavigationSensors() = default; 

  struct NavigationSnapshot {
    SensorSample<uint16_t> compass_hdg_dg;
    SensorSample<uint16_t> gps_cog_dg;
    SensorSample<float> gps_sog_kts;
    SensorSample<uint16_t> wind_angle_dg;
    SensorSample<float> stw_kts;
  };

  struct SensorActivation {
    bool compass = false;
    bool gps = false;
    bool wind = false;
    // NOTE:
    // STW (Speed Through Water) is always sampled when available via NMEA.
    // It is not a selectable navigation source, but a context parameter
    // for impulse modulation. Therefore it is not part of SensorActivation.
  };

  NavigationSnapshot createSnapshot();
  void setSensorActivations(NavigationSource source, bool setTo);
  void setLeadSource(NavigationSource src);
  NavigationSource getLeadSource() const;

private:
  NavigationSource m_leadSource = NavigationSource::Compass;
  CompassModule m_compass;
  GPSModule m_gps;
  WindModule m_wind;
  NMEA183BUS m_nmea183Bus;
  SensorActivation m_sensorActivation;
};

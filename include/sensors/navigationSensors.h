#pragma once
#include "sensors/NMEA183BUS.h"
#include "sensors/compassModule.h"
#include "sensors/gpsModule.h"
#include "sensors/windModule.h"
#include "types/globalTypes.h"
#include <cstdint>

class NavigationSensors {
public:
  explicit NavigationSensors(const CompassModule &compass, const GPSModule &gps,
                             const WindModule &wind,
                             const NMEA183BUS &nmea183Bus)
      : m_compass(compass), m_gps(gps), m_wind(wind),
        m_nmea183Bus(nmea183Bus) {};

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
  const CompassModule &m_compass;
  const GPSModule &m_gps;
  const WindModule &m_wind;
  const NMEA183BUS &m_nmea183Bus;
  SensorActivation m_sensorActivation;
};

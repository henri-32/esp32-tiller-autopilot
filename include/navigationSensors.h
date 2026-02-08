#pragma once
#include "NMEA183BUS.h"
#include "compassModule.h"
#include "globalTypes.h"
#include "gpsModule.h"
#include "windModule.h"
#include <cstdint>
#include <optional>


class NavigationSensors {
public:
  explicit NavigationSensors(const CompassModule &compass, const GPSModule &gps,
                             const WindModule &wind,
                             const NMEA183BUS &nmea183Bus)
      : m_compass(compass),
        m_gps(gps),
        m_wind(wind),
        m_nmea183Bus(nmea183Bus) {};

  void setActiveSource(NavigationSource src);
  NavigationSource getActiveSource() const;

  std::optional<uint16_t> getCurrentReading() const;
  std::optional<float> getSOG() const;

  bool sensorValue_valid() const;

private:
  NavigationSource m_activeSource = NavigationSource::Compass;
  const CompassModule &m_compass;
  const GPSModule &m_gps;
  const WindModule &m_wind;
  const NMEA183BUS &m_nmea183Bus;
};

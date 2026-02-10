#pragma once
#include "sensors/NMEA183BUS.h"
#include "sensors/compassModule.h"
#include "sensors/gpsModule.h"
#include "sensors/windModule.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <optional>

class NavigationSensors {
public:
  enum class SourceSwitchReason {
    ManualSelection,
    QualityDegraded,
    SourceUnavailable,
  };
  explicit NavigationSensors(const CompassModule &compass, const GPSModule &gps,
                             const WindModule &wind,
                             const NMEA183BUS &nmea183Bus)
      : m_compass(compass), m_gps(gps), m_wind(wind),
        m_nmea183Bus(nmea183Bus) {};

  void setActiveSource(NavigationSource src);
  NavigationSource getActiveSource() const;

  std::optional<uint16_t> getCurrentReading() const;
  std::optional<float> getSOG() const;

  bool sensorValueIsvalid() const;

private:
  NavigationSource m_activeSource = NavigationSource::Compass;
  const CompassModule &m_compass;
  const GPSModule &m_gps;
  const WindModule &m_wind;
  const NMEA183BUS &m_nmea183Bus;
};

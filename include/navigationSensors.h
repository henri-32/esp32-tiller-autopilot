#pragma once
#include "compassModule.h"
#include "globalTypes.h"
#include "gpsModule.h"
#include "windModule.h"
#include <cstdint>
#include <optional>


class NavigationSensors {
public:
  explicit NavigationSensors(const CompassModule &compass, const GPSModule &gps,
                             const WindModule &wind)
      : m_compass(compass), m_gps(gps), m_wind(wind) {};

  void setActiveSource(NavigationSource src);
  NavigationSource getActiveSource();

  std::optional<uint16_t> getCurrentReading() const;

private:
  NavigationSource m_activeSource = NavigationSource::Compass;
  const CompassModule &m_compass;
  const GPSModule &m_gps;
  const WindModule &m_wind;
};

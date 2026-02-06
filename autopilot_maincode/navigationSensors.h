#pragma once
#include "compassModule.h"
#include "gpsModule.h"
#include "windModule.h"
#include <Arduino.h>

class NavigationSensors {
public:
  enum class ActiveSource { Compass, Wind, Gps };

  explicit NavigationSensors(const CompassModule &compass, const GPSModule &gps,
                             const WindModule &wind)
      : m_compass(compass), m_gps(gps), m_wind(wind) {};

  void setActiveSource(ActiveSource src);

  uint16_t getCurrentReading() const;

private:
  ActiveSource m_activeSource = ActiveSource::Compass;
  const CompassModule &m_compass;
  const GPSModule &m_gps;
  const WindModule &m_wind;
};
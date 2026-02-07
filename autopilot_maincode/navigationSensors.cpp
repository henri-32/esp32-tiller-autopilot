#include "navigationSensors.h"
#include <optional>
#include <Arduino.h>
#include <cstdlib>


void NavigationSensors::setActiveSource(NavigationSource src) {
  m_activeSource = src;
};
NavigationSource NavigationSensors::getActiveSource(){return m_activeSource;}

std::optional<uint16_t> NavigationSensors::getCurrentReading() const {
  switch (m_activeSource) {
  case NavigationSource::Compass:
    return m_compass.readHeading();
  case NavigationSource::Wind:
    return m_wind.readHeading();
  case NavigationSource::Gps:
    return m_gps.readHeading();
  default:
  return std::nullopt;
  }
};
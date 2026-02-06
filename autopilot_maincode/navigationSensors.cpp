#include "navigationSensors.h"
#include <cstdint>

void NavigationSensors::setActiveSource(ActiveSource src) {
  m_activeSource = src;
};

std::optional<uint16_t> NavigationSensors::getCurrentReading() const {
  switch (m_activeSource) {
  case ActiveSource::Compass:
    return m_compass.readHeading();
  case ActiveSource::Wind:
    return m_wind.readHeading();
  case ActiveSource::Gps:
    return m_gps.readHeading();
  }
};
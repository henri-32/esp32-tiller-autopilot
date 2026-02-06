#include "navigationSensors.h"


void NavigationSensors::setActiveSource(NavigationSource src) {
  m_activeSource = src;
};

std::optional<uint16_t> NavigationSensors::getCurrentReading() const {
  switch (m_activeSource) {
  case NavigationSource::Compass:
    return m_compass.readHeading();
  case NavigationSource::Wind:
    return m_wind.readHeading();
  case NavigationSource::Gps:
    return m_gps.readHeading();
  }
};
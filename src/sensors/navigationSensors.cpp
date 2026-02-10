#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>
#include <optional>

void NavigationSensors::setActiveSource(NavigationSource src) {
  m_activeSource = src;
};
NavigationSource NavigationSensors::getActiveSource() const {
  return m_activeSource;
}

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

std::optional<float> NavigationSensors::getSOG() const {
  return m_gps.readSOG();
};

bool NavigationSensors::sensorValueIsvalid() const{
  switch (m_activeSource) {
  case NavigationSource::Gps:
    return m_gps.isValue_valid();
  case NavigationSource::Compass:
    return m_compass.isValue_valid(); // In dem Fall muss an irgendeiner Stelle
                                      // definitiv Error Handling erfolgen
  case NavigationSource::Wind:
    return m_wind.isValue_valid();
  default:
    return false;
  }
};

#pragma once
#include "drivers/NASA_Duo.h"
#include "drivers/compassDriver.h"
#include "drivers/gpsDriver.h"
#include "drivers/windModule.h"
#include "types/sensorTypes.h"
#include "types/controllerTypes.h"
#include "types/sensorTypes.h"
#include <cstdint>

class NavigationSensors {
public:
  NavigationSensors() = default;


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
  // TODO(Architektur):
  // SensorActivation spaeter in createSnapshot() wirksam machen
  // (z. B. Sensor gezielt nicht lesen), damit Modi tatsaechlich
  // Energie sparen und nicht nur Status-Flags setzen.
  void setSensorActivations(NavigationSource source, bool setTo);
  void setLeadSource(NavigationSource src);
  NavigationSource getLeadSource() const;

private:
  uint16_t compass_sensor_fusion(); 
  NavigationSource m_leadSource = NavigationSource::Compass;
  CompassDriver m_compass;
  GpsDriver m_gps;
  WindModule m_wind;
  NASA_Duo m_nmea183Bus;
  SensorActivation m_sensorActivation;
};

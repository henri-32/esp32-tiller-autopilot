#include "sensors/NMEA183BUS.h"

SensorSample<float> NMEA183BUS::readSTW() const {
  SensorSample<float> sample;
  sample.value = 0.0f;
  sample.valid = false;
  return sample;
}

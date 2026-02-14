#include "sensors/gpsModule.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <sys/types.h>

SensorSample<uint16_t> GPSModule::readCOG() const {
  SensorSample<uint16_t> sample;

  const uint16_t raw = 0; /*Hardware lesen*/
  if (true /*Plausibilitätscheck*/) {
    sample.value = raw;
    sample.valid = true;
  } else if (false /*Plausi falsch*/) {
    sample.value = raw;
    sample.valid = false;
  };
  return sample;
};

SensorSample<float> GPSModule::readSOG() const {
  SensorSample<float> sample;

  const float raw = 0.0f; // Hardware lesen
  if (true /*Plausi checken*/) {
    sample.value = raw;
    sample.valid = true;
  } else if (false /*Plausi falsch*/) {
    sample.value = raw;
    sample.valid = false;
  };

  return sample;
};

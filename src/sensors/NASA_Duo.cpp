#include "sensors/NASA_Duo.h"

SensorSample<float> NASA_Duo::readSTW() const {
  SensorSample<float> sample;
  sample.value = 0.0f;
  sample.valid = false;
  return sample;
}

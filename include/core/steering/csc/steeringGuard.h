#pragma once

#include "core/config.h"
#include <cstdint>

class SteeringGuard {
public:
  SteeringGuard(SteeringController_Config &config);

  bool activeObservationGuard(uint32_t loopTimestamp, uint32_t lastUpdate,
                              uint32_t lastIntent);
  bool activeIntentGuard(uint32_t loopTimestamp, uint32_t lastIntent,
                         uint8_t sampleSize);

private:
  SteeringController_Config &m_config;
};

#pragma once

#include "core/config.h"
#include "core/steering/csc/deadband.h"
#include <cstdint>

class SteeringGuard {
public:
  // Contract:
  // Purpose: Gate observation/intent by timing and sample thresholds.
  // Inputs: timestamps, sample size, regulation config slice.
  // Outputs/Side-effects: boolean decisions, no side-effects.
  SteeringGuard(SteeringRegulationConfig &config);

  bool observationBlocked(uint32_t loopTimestamp, uint32_t lastUpdate,
                          uint32_t lastIntent);
  bool intentBlocked(uint32_t loopTimestamp, uint32_t lastIntent,
                     uint8_t sampleSize, int16_t median);

private:
  SteeringRegulationConfig &m_config;
  Deadband m_deadband;
};

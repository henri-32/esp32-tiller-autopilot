#pragma once

#include "core/config.h"
#include <array>
#include <cstdint>

class ObservationBuffer {
private:
  SteeringRegulationConfig &m_config;

  static constexpr uint8_t bufferSize =
      SteeringRegulationConfig::observationBufferSize;

  std::array<int16_t, bufferSize> m_errorArray{};
  uint8_t writtenErrorsCounter = 0;
  uint8_t medianIndex;
  int16_t median = 0;

public:
  // Contract:
  // Purpose: Store recent errors and compute a median for direction decision.
  // Inputs: error samples + regulation config slice.
  // Outputs/Side-effects: median and sample count; internal buffer mutated.
  ObservationBuffer(SteeringRegulationConfig &config);

  void update(int16_t error);
  int16_t getMedian() const;
  int16_t getSmoothedCurrentError(); 
  uint8_t getSampleSize() const;
  void reset();
};

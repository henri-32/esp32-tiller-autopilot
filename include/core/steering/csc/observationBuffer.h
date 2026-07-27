#pragma once

#include "config/config.h"
#include <array>
#include <cstdint>

class ObservationBuffer {
private:
  const SteeringRegulationConfig &m_config;

  static constexpr uint8_t maxBufferSize =
      SteeringRegulationConfig::observationBufferSize;

  uint8_t getActiveBufferSize() const;

  std::array<int16_t, maxBufferSize> m_medianArray{};
  std::array<int16_t, maxBufferSize> m_meanArray{};
  std::array<uint32_t, maxBufferSize> m_timeArray{};
  uint8_t writtenErrorsCounter = 0;
  uint8_t medianIndex;
  int16_t median = 0;


public:
  // Contract:
  // Purpose: Store recent errors and compute a median for direction decision.
  // Inputs: error samples + regulation config slice.
  // Outputs/Side-effects: median and sample count; internal buffer mutated.
  ObservationBuffer(const SteeringRegulationConfig &config);

  void update(int16_t error);
  void update(int16_t error, uint32_t loopTimestamp);
  int16_t getMedian() const;
  int16_t getSmoothedCurrentError(); 
  uint8_t getSampleSize() const;
  float getOmega(uint32_t loopTimestamp) const; 
  void reset();
};

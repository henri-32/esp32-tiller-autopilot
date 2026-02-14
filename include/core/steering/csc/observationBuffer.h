#pragma once

#include "core/config.h"
#include <array>
#include <cstdint>

class ObservationBuffer {
private:
  SteeringController_Config &m_config;

  static constexpr uint8_t bufferSize =
      SteeringRegulationConfig::observationBufferSize;

  std::array<int16_t, bufferSize> m_errorArray;
  uint8_t validErrorsCounter = 0;
  uint8_t medianIndex;
  int16_t median = 0;

public:
  ObservationBuffer(SteeringController_Config &config);

  const int16_t getMedian() const;
  void update(int16_t error);
  uint8_t getSampleSize() const; 
  void reset();
};

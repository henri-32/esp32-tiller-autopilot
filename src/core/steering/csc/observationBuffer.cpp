#include "core/steering/csc/observationBuffer.h"

#include <algorithm>

ObservationBuffer::ObservationBuffer(SteeringRegulationConfig &config)
    : m_config(config) {}

int16_t ObservationBuffer::getMedian() const { return median; }

uint8_t ObservationBuffer::getActiveBufferSize() const {
  const uint8_t configured = m_config.activeObservationBufferSize;
  if (configured == 0) {
    return 1;
  }
  return std::min(configured, maxBufferSize);
}

void ObservationBuffer::update(int16_t error) {
  const uint8_t activeBufferSize = getActiveBufferSize();

  // Reset the buffer if it overflows its active capacity.
  if (writtenErrorsCounter >= activeBufferSize) {
    reset();
  }

  m_medianArray[writtenErrorsCounter] = error;
  m_meanArray[writtenErrorsCounter] = error;
  writtenErrorsCounter++;

  // Deliberately no ring buffer: after one evidence window, values are dropped.
  std::sort(m_medianArray.begin(), m_medianArray.begin() + writtenErrorsCounter);

  medianIndex = writtenErrorsCounter / 2;
  median = m_medianArray[medianIndex];

  // Symmetric guard: if positives and negatives are balanced, treat as zero.
  uint8_t negativeValues = 0;
  uint8_t positiveValues = 0;
  for (uint8_t i = 0; i < writtenErrorsCounter; i++) {
    if (m_medianArray[i] < 0) {
      negativeValues++;
    } else if (m_medianArray[i] > 0) {
      positiveValues++;
    }
  }

  if (positiveValues == negativeValues) {
    median = 0;
  }

  // Full window consumed: reset evidence intentionally.
  if (writtenErrorsCounter >= activeBufferSize) {
    reset();
  }
}

int16_t ObservationBuffer::getSmoothedCurrentError() {
  int32_t sum = 0;
  const uint16_t samples = writtenErrorsCounter;
  if (samples == 0) {
    return 0;
  }

  const uint8_t cfgWindow = m_config.calculationWindowSmoothedMean;
  if (cfgWindow == 0) {
    return 0;
  }

  const uint16_t window =
      std::min(static_cast<uint16_t>(samples), static_cast<uint16_t>(cfgWindow));
  const uint16_t start = samples - window;

  for (uint16_t i = start; i < samples; i++) {
    sum += m_meanArray[i];
  }

  return static_cast<int16_t>(sum / static_cast<int32_t>(window));
}

uint8_t ObservationBuffer::getSampleSize() const { return writtenErrorsCounter; }

void ObservationBuffer::reset() {
  for (uint8_t i = 0; i < maxBufferSize; i++) {
    m_medianArray[i] = 0;
    m_meanArray[i] = 0;
  }
  writtenErrorsCounter = 0;
  medianIndex = 0;
  median = 0;
}

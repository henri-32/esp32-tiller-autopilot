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

void ObservationBuffer::update(int16_t error, uint32_t loopTimestamp) {
  const uint8_t activeBufferSize = getActiveBufferSize();

  // Reset the buffer if it overflows its active capacity.
  if (writtenErrorsCounter >= activeBufferSize) {
    reset();
  }

  m_medianArray[writtenErrorsCounter] = error;
  m_meanArray[writtenErrorsCounter] = error;
  m_timeArray[writtenErrorsCounter] = loopTimestamp;
  writtenErrorsCounter++;

  // Deliberately no ring buffer: after one evidence window, values are dropped.
  std::sort(m_medianArray.begin(),
            m_medianArray.begin() + writtenErrorsCounter);

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
  const uint8_t samples = writtenErrorsCounter;
  if (samples == 0) {
    return 0;
  }

  const uint8_t cfgWindow = m_config.calculationWindowSmoothedMean;
  if (cfgWindow == 0) {
    return 0;
  }

  const uint8_t window = std::min(samples, cfgWindow);
  const uint8_t start = samples - window;

  for (uint8_t i = start; i < samples; i++) {
    sum += m_meanArray[i];
  }

  return static_cast<int16_t>(sum / window);
}

uint8_t ObservationBuffer::getSampleSize() const {
  return writtenErrorsCounter;
}

void ObservationBuffer::reset() {
  for (uint8_t i = 0; i < maxBufferSize; i++) {
    m_medianArray[i] = 0;
    m_meanArray[i] = 0;
    m_timeArray[i] = 0;
  }
  writtenErrorsCounter = 0;
  medianIndex = 0;
  median = 0;
}

float ObservationBuffer::getOmega(uint32_t loopTimestamp) const {
  (void)loopTimestamp;

  const uint8_t samples = writtenErrorsCounter;
  if (samples < 2) {
    return 0.0f;
  }

  const uint8_t cfgWindow = m_config.omegaRobust;
  if (cfgWindow == 0) {
    return 0.0f;
  }

  const uint8_t window = std::min(samples, cfgWindow);
  if (window == 0) {
    return 0.0f;
  }

  if (window < 2) {
    return 0.0f;
  }

  const uint8_t start = samples - window;
  const uint8_t end = samples - 1;

  const uint32_t tStart = m_timeArray[start];
  const uint32_t tEnd = m_timeArray[end];
  if (tEnd <= tStart) {
    return 0.0f;
  }

  const uint32_t duration_ms = tEnd - tStart;
  if (duration_ms == 0) {
    return 0.0f;
  }

  const int32_t deltaError = static_cast<int32_t>(m_meanArray[end]) -
                             static_cast<int32_t>(m_meanArray[start]);
  const float errorRate_deg_s = (static_cast<float>(deltaError) * 1000.0f) /
                                static_cast<float>(duration_ms);

  // Sign convention for guard semantics:
  // positive omega means error decreases for positive median (moving toward
  // target).
  const float omega_deg = -errorRate_deg_s;
  return omega_deg;
}

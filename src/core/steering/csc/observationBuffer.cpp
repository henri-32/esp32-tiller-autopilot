#include "core/steering/csc/observationBuffer.h"
#include <algorithm>

ObservationBuffer::ObservationBuffer(SteeringRegulationConfig &config)
    : m_config(config) {};

const int16_t ObservationBuffer::getMedian() const { return median; };

void ObservationBuffer::update(int16_t error) {
  // Reset the buffer if it overflows its configured capacity.
  if (validErrorsCounter >= m_config.observationBufferSize) {
    reset();
  }

  m_errorArray[validErrorsCounter] = error;
  validErrorsCounter++;

  /*Das Array ist bewusst kein Ringbuffer, da die Ereignisse des letzten Arrays
    semantisch bewusst verworfen werden*/
  std::sort(m_errorArray.begin(), m_errorArray.begin() + validErrorsCounter);

  /* Da nur signifikante Werte geschrieben werden, ist null semantisch kein Wert
  statt gemessen 0 Da in jedem update neue Medianberechnung ist kein
  Zurücksetzen erforderlich*/
  medianIndex = validErrorsCounter / 2;
  median = m_errorArray[medianIndex];

  // Symmetric guard: if positives and negatives are balanced, treat as zero.
  bool symmetric = false;
  uint8_t negativeValues = 0;
  uint8_t positiveValues = 0;
  for (int i = 0; i < validErrorsCounter; i++) {
    if (m_errorArray[i] < 0) {
      negativeValues++;
    } else if (m_errorArray[i] > 0) {
      positiveValues++;
    }
  }
  if (positiveValues == negativeValues) {
    symmetric = true;
  }

  if (symmetric) {
    median = 0;
  }
}

uint8_t ObservationBuffer::getSampleSize() const { return validErrorsCounter; };

/*siehe oben bewusst verworfen*/
void ObservationBuffer::reset() {
  for (int i = 0; i < bufferSize; i++) {
    m_errorArray[i] = 0;
  }
  validErrorsCounter = 0;
  medianIndex = 0;
  median = 0;
}

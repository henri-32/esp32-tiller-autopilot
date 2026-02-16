#include "core/steering/csc/observationBuffer.h"
#include <algorithm>

ObservationBuffer::ObservationBuffer(SteeringRegulationConfig &config)
    : m_config(config) {};

int16_t ObservationBuffer::getMedian() const { return median; };

void ObservationBuffer::update(int16_t error) {
  // Reset the buffer if it overflows its configured capacity.
  if (writtenErrorsCounter >= m_config.observationBufferSize) {
    reset();
  }

  m_errorArray[writtenErrorsCounter] = error;
  writtenErrorsCounter++;

  /*Das Array ist bewusst kein Ringbuffer, da die Ereignisse des letzten Arrays
    semantisch bewusst verworfen werden*/
  std::sort(m_errorArray.begin(), m_errorArray.begin() + writtenErrorsCounter);

  /* Da nur signifikante Werte geschrieben werden, ist null semantisch kein Wert
  statt gemessen 0 Da in jedem update neue Medianberechnung ist kein
  Zurücksetzen erforderlich*/
  medianIndex = writtenErrorsCounter / 2;
  median = m_errorArray[medianIndex];

  // Symmetric guard: if positives and negatives are balanced, treat as zero.
  bool symmetric = false;
  uint8_t negativeValues = 0;
  uint8_t positiveValues = 0;
  for (int i = 0; i < writtenErrorsCounter; i++) {
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

int16_t ObservationBuffer::getSmoothedCurrentError() {
  int16_t sum = 0;
  int8_t counter = 0;

  /*Die letzten Errors werden für mean Berechnung genutzt ; Anzahl je nach
    config*/
  for (int i = 0;
       i < writtenErrorsCounter - m_config.calculationWindowSmoothedMean; i++) {
    counter++;
    sum += m_errorArray[i];
  };
  int8_t mean = sum / counter;
  return mean;
};

uint8_t ObservationBuffer::getSampleSize() const {
  return writtenErrorsCounter;
};

/*siehe oben bewusst verworfen*/
void ObservationBuffer::reset() {
  for (int i = 0; i < bufferSize; i++) {
    m_errorArray[i] = 0;
  }
  writtenErrorsCounter = 0;
  medianIndex = 0;
  median = 0;
}

#include "core/steering/csc/observationBuffer.h"

ObservationBuffer::ObservationBuffer(SteeringController_Config &config)
    : m_config(config) {};

const int16_t ObservationBuffer::getMedian() const { return median; };

void ObservationBuffer::update(int16_t error) {
  m_errorArray[validErrorsCounter] = error;
  validErrorsCounter++;

  /*Das Array ist bewusst kein Ringbuffer, da die Ereignisse des letzten Arrays
    semantisch bewusst verworfen werden*/
  if (validErrorsCounter >= m_config.regulations.observationBufferSize) {
    for (int i = 0; i == bufferSize; i++) {
      m_errorArray[i] = 0;
      validErrorsCounter = 0;
    };
  };

  /* Da nur signifikante Werte geschrieben werden, ist null semantisch kein Wert
  statt gemessen 0 Da in jedem update neue Medianberechnung ist kein
  Zurücksetzen erforderlich*/
  medianIndex = validErrorsCounter / 2;
  median = m_errorArray[medianIndex];
};

uint8_t ObservationBuffer::getSampleSize() const { return validErrorsCounter; };

/*siehe oben bewusst verworfen*/
void ObservationBuffer::reset() {
  for (int i = 0; i < bufferSize; i++) {
    m_errorArray[i] = 0;
    validErrorsCounter = 0;
  };
};

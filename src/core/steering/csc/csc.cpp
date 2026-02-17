#include "core/steering/csc/csc.h"

#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include <cstdint>
#include <cmath> 

CoreSteeringController::CoreSteeringController(SteeringRegulationConfig &config)
    : m_observationBuffer(config), m_deadband(config), m_steeringGuard(config),
      m_config(config) {}

std::optional<SteeringIntent>
CoreSteeringController::tick(uint32_t loopTimestamp) {
  // Debug flags are per-tick state, not latched state.
  m_debug.observationBlocked = false;
  m_debug.intentBlocked = false;
  m_debug.deadbandActive = false;

  /*CSC macht zwei Sachen
  - Beobachten
  - Intent zurückgeben

  Wenn er wegen Guards nicht beobachten darf early return*/
  if (m_steeringGuard.observationBlocked(loopTimestamp, m_lastObsUpdate,
                                         m_lastIntent)) {
    m_debug.observationBlocked = true;
    return std::nullopt;
  }

  SteeringIntent intent; // Return value

  int16_t error = m_errorCalculator.getCurrentError(m_currentCourse,
                                                    m_internalTargetCourse);
  m_debug.error = error;

  m_observationBuffer.update(error);
  m_lastObsUpdate = loopTimestamp;

  m_debug.median = m_observationBuffer.getMedian();
  m_debug.sampleSize = m_observationBuffer.getSampleSize();

  if (m_steeringGuard.intentBlocked(loopTimestamp, m_lastIntent,
                                    m_observationBuffer.getSampleSize(), m_observationBuffer.getMedian())) {
    m_debug.intentBlocked = true;

    return std::nullopt;
  } else {

    auto intent = calculateIntentFromObs();
    m_lastIntent = loopTimestamp;

    /* Wenn Handlung ausgelöst wird, wurde auf Evidenz reagiert und
      diese wird bewusst verworfen*/
    m_observationBuffer.reset();

    return intent;
  }
}

void CoreSteeringController::currentHDG(uint16_t current) {
  m_currentCourse = current;
}

void CoreSteeringController::setInternalTarget(uint16_t target) {
  m_internalTargetCourse = target;
};

uint16_t CoreSteeringController::getInternalTarget() const {
  return m_internalTargetCourse;
};

std::optional<SteeringIntent> CoreSteeringController::calculateIntentFromObs() {

  /* Der Median wird als robuster Mittelwert für die Entscheidung
  Action/NoAction und Richtungsentscheidung genutzt. Ein leicht geglätteter
  aktueller mean wird als Fehlergröße für die Berechnung der abstrakten
  Impulsstärke genutzt. */

  SteeringIntent intent; // return value

  intent.dir = determineDirection(m_observationBuffer.getMedian());

  /* Semantisch klargestellt. CSC setzt grundsätzlich bei jeder Entscheidung
  vollen abstrakten Impuls.
  Der kann danach nach unten gedämpft, aber nicht nach
  oben eskaliert werden.*/
  intent.abstractImpulse_0_100 = 100;
  int8_t currentError = m_observationBuffer.getSmoothedCurrentError();

  if (currentError < - m_config.smoothedMeanApplicationWindow_deg ||
      currentError > m_config.smoothedMeanApplicationWindow_deg) {
    return intent;
  };

  /*Bis hier hin war Richtung noch wichtig, um mean korrekt bestimmen zu
  können Ab hier nur noch für abstractImpulse Größe ohne Vorzeichenrelevanz
  verwendet*/
  currentError = std::abs(currentError);

  /*Wenn die obere (positive) Grenze des Windows 100% abstract Impulse bedeutet,
  wird so in linearem Verhältnis auf die Fehlergröße reagiert*/
  float errorPercantage = static_cast<float>(currentError) /
                          m_config.smoothedMeanApplicationWindow_deg;
  intent.abstractImpulse_0_100 *= errorPercantage;

  return intent;
};

SteeringDirection
CoreSteeringController::determineDirection(int16_t median) const {
  if (median < 0) {
    return SteeringDirection::Left;
  } else if (median > 0) {
    return SteeringDirection::Right;
  };
  return SteeringDirection::Left;
}

const CSCDebugSnapshot &CoreSteeringController::getDebug() const {
  return m_debug;
}

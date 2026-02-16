#include "core/steering/csc/csc.h"

#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include <algorithm>
#include <cstdint>

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

  SteeringIntent intent; // Return value instanziert

  int16_t error = m_errorCalculator.getCurrentError(m_currentCourse,
                                                    m_internalTargetCourse);
  m_debug.error = error;

  /*
  Aktuell ist das ganze Deadband Modul nicht genutzt. Siehe
  documentation\csc_simulation_tests\iterations\1.
  additional_csc_paramteter_for_error_size

  -------------------------------------------
    if (!m_deadband.errorSignificant(error)) {
    m_debug.deadbandActive = true;
    return std::nullopt;
  }
  -----------------------------------------------
*/

  m_observationBuffer.update(error);

  m_debug.median = m_observationBuffer.getMedian();
  m_debug.sampleSize = m_observationBuffer.getSampleSize();

  m_lastObsUpdate = loopTimestamp;

  if (m_steeringGuard.intentBlocked(loopTimestamp, m_lastIntent,
                                    m_observationBuffer.getSampleSize())) {
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

  /* Der Median wird bewusst nur für die Bestimmung der
  nötigen Korrekturrichtung genutzt, da ich bewusst keinen
  PID Regler will*/
  SteeringIntent intent;

  intent.dir = determineDirection(m_observationBuffer.getMedian());

  /* Semantisch klargestellt. CSC setzt vollen abstrakten Impuls.
  Der kann danach nur nach unten gedämpft, aber nicht nach
  oben eskaliert werden.*/
  intent.abstractImpulse_0_100 = 100;
  int8_t currentError = m_observationBuffer.getSmoothedCurrentError();

  /*In dem clamped Fenster wird der Abstrakte Impulse runter gefiltert
  Außerhalb bleibt er voll */
  currentError = std::clamp(
      currentError,
      static_cast<int8_t>(m_config.lowClampSmoothedMeanApplicationWindow_deg),
      static_cast<int8_t>(m_config.highClampSmoothedMeanApplicationWindow_deg));

  int8_t filter = TODO WEITERARBEITEN Filter entwickeln Außerdem die neuen
      config werte auch für sim anwenden !!return intent;
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

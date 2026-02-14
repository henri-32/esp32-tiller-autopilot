#include "core/steering/csc/csc.h"

#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include <cstdint>

CoreSteeringController::CoreSteeringController(SteeringRegulationConfig &config)
    :  m_observationBuffer(config), m_deadband(config),
      m_steeringGuard(config) {}

std::optional<SteeringIntent>
CoreSteeringController::tick(uint32_t loopTimestamp) {
  /*CSC macht zwei Sachen
  - Beobachten
  - Intent zurückgeben

  Wenn er wegen Guards nicht beobachten darf early return*/

  // Gate observation to avoid reacting too frequently.
  if (m_steeringGuard.observationBlocked(loopTimestamp, m_lastObsUpdate,
                                         m_lastIntent)) {
    return std::nullopt;
  }

  SteeringIntent intent; // Return value

  int16_t error = m_errorCalculator.getCurrentError(m_currentCourse,
                                                    m_internalTargetCourse);

  /*Ein nicht signifikanter Error kann keine Aktion auslösen
  Deswegen darf das direkt zum early return führen*/
  // Ignore small errors to avoid actuator chatter.
  if (!m_deadband.errorSignificant(error)) {
    return std::nullopt;
  }

  m_observationBuffer.update(error);
  m_lastObsUpdate = loopTimestamp;

  /* Der Median wird bewusst nur für die Bestimmung der
  nötigen Korrekturrichtung genutzt, da ich bewusst keinen
  PID Regler will*/

  SteeringDirection dir = determineDirection(m_observationBuffer.getMedian());

  if (m_steeringGuard.intentBlocked(loopTimestamp, m_lastIntent,
                                    m_observationBuffer.getSampleSize())) {
    return std::nullopt;
  } else {
    intent.dir = dir;

    /* Semantisch klargestellt. CSC setzt vollen abstrakten Impuls.
    Der kann danach nur nach unten gedämpft, aber nicht nach
    oben eskaliert werden.*/

    intent.abstractImpulse_0_100 = 100;
    m_lastIntent = loopTimestamp;

    /* Wenn Handlung ausgelöst wird, wurde auf Evidenz reagiert und
    diese wird bewusst verworfen*/

    // Intent consumed; discard accumulated evidence to start fresh.
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

SteeringDirection
CoreSteeringController::determineDirection(int16_t median) const {
  if (median < 0) {
    return SteeringDirection::Left;
  } else if (median > 0) {
    return SteeringDirection::Right;
  };
  return SteeringDirection::Left;
}

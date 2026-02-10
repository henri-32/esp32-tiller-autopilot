#include "core/steeringController.h"
#include "actuators/pwmController.h"
#include "diagnostics/diagnostic_types.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"

#include <algorithm>
#include <array>
#include <cstdint>

SteeringController::SteeringController(PWMController &pwm,
                                       NavigationSensors &navsens,
                                       SteeringController_Config &config,
                                       Diagnostics &diagnostics)
    : m_csc(pwm, navsens, config), m_diagnostics(diagnostics) {};

void SteeringController::setCurrentCompassCourse(uint16_t current) {
  m_csc.setCurrentCompassCourse(current);
}

void SteeringController::tick(unsigned long loopTimestamp) {
  m_csc.tick(loopTimestamp);
}

SteeringController::SourceEvaluator::SourceEvaluator(
    NavigationSensors &navsens, SteeringController_Config &config,
    Diagnostics &diagnostics)
    : m_navigationSensors(navsens), m_steeringController_Config(config),
      m_diagnostics(diagnostics) {};

void SteeringController::SourceEvaluator::tick(unsigned long loopTimestamp) {
  switch (m_navigationSensors.getActiveSource()) {
  case NavigationSource::Gps:
    if (!m_navigationSensors.sensorValueIsvalid()) {
      m_navigationSensors.setActiveSource(NavigationSource::Compass);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::GPS, loopTimestamp);

      // Behandlung geschwindigkeit zu gering für Regelung
      // Alles noch in einzelne Funktion auslagern
    };
    break;

  case NavigationSource::Compass:
    if (!m_navigationSensors.sensorValueIsvalid()) {
      m_navigationSensors.setActiveSource(NavigationSource::Gps);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::COMPASS, loopTimestamp);
      // SystemController muss noch auswerten, wie lange lost, damit dann
      // critical Error. Das Fallback kann nur übergangsweise erfolgen.
    };
    break;

  case NavigationSource::Wind:
    if (!m_navigationSensors.sensorValueIsvalid()) {
      m_navigationSensors.setActiveSource(NavigationSource::Compass);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback, FunctionalCapability::WIND, loopTimestamp);
    }
    break;
  };
};

SteeringController::CoreSteeringController::CoreSteeringController(
    PWMController &pwm, NavigationSensors &navsens,
    SteeringController_Config &config)
    : m_pwm(pwm), m_navigationSensors(navsens),
      m_SteeringController_Config(config) {}

void SteeringController::CoreSteeringController::tick(
    unsigned long loopTimestamp) {
  if (observationAllowed(loopTimestamp)) {
    updateObservationBuffers(loopTimestamp);
  }

  if (actionAllowed(loopTimestamp))
    if (steeringCorrectionIsRequired()) {
      m_pwm.command(calculateSteeringDirectionFromObservation());
      m_lastImpulse = loopTimestamp;
      resetObservations();
    }
}

void SteeringController::CoreSteeringController::setCurrentCompassCourse(
    uint16_t current) {
  m_currentCourse = current;
}

bool SteeringController::CoreSteeringController::observationAllowed(
    unsigned long loopTimestamp) const {
  if (loopTimestamp - m_lastImpulse >
      m_SteeringController_Config.pauseForValidObsAfterImpulse_ms) {
    return true;
  } else
    return false;
};

void SteeringController::CoreSteeringController::updateObservationBuffers(
    unsigned long loopTimestamp) {
  if (loopTimestamp - m_lastObservation <
      m_SteeringController_Config.minimumTimeBtwObs_ms)
    return;

  uint16_t currentError =
      calculateHeadingError(m_currentCourse, m_internalTargetCourse);
  if (currentError > 0) {
    m_leftErrors[m_observationCount] = 0;
    m_rightErrors[m_observationCount] = currentError;
  } else if (currentError < 0) {
    m_rightErrors[m_observationCount] = 0;
    m_leftErrors[m_observationCount] = currentError;
  }

  m_mergedError[m_observationCount] =
      m_rightErrors[m_observationCount] + m_leftErrors[m_observationCount];
  m_observationCount++;
  m_lastObservation = loopTimestamp;

  if (m_observationCount >= m_SteeringController_Config.observationBufferSize) {
    resetObservations();
  }
};

bool SteeringController::CoreSteeringController::actionAllowed(
    unsigned long loopTimestamp) const {
  if (loopTimestamp - m_lastImpulse >
      m_SteeringController_Config.SteeringCooldown_ms) {
    return true;
  } else
    return false;
};

bool SteeringController::CoreSteeringController::
    steeringCorrectionIsRequired() {
  auto medianArray = m_mergedError;
  std::sort(medianArray.begin(), medianArray.end());

  uint8_t medianIndex =
      m_SteeringController_Config.observationBufferSize /
      2; // hier nur der upper median bei geraden Mengen von Werten
  m_medianOfMergedErrors = medianArray[medianIndex];

  if (abs(m_medianOfMergedErrors) >=
      m_SteeringController_Config.steeringTolerance_deg) {
    return true;
  }
  return false;
};

SteeringDirection SteeringController::CoreSteeringController::
    calculateSteeringDirectionFromObservation() {
  if (m_medianOfMergedErrors <= 0) {
    return SteeringDirection::Left;
  } else if (m_medianOfMergedErrors > 0) {
    return SteeringDirection::Right;
  }
  return SteeringDirection::Left;
};

void SteeringController::CoreSteeringController::resetObservations() {
  m_observationCount = 0;
  for (int i = 0; i <= 99; i++) {
    m_leftErrors[i] = 0;
    m_rightErrors[i] = 0;
    m_mergedError[i] = 0;
  }
};

int16_t SteeringController::CoreSteeringController::calculateHeadingError(
    uint16_t current, uint16_t target) {
  int16_t diff = static_cast<int16_t>(target) - static_cast<int16_t>(current);

  diff += 360;
  diff += 180;
  diff %= 360;
  diff -= 180;

  return diff;
}

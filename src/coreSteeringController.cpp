#include "coreSteeringController.h"
#include "globalTypes.h"
#include "pwmController.h"

#include <Arduino.h>
#include <algorithm>
#include <array>
#include <cstdint>

CoreSteeringController::CoreSteeringController(PWMController &pwm,
                                               NavigationSensors &navsens,
                                               SystemConfig &config)
    : m_pwm(pwm), m_navigationSensors(navsens), m_systemConfig(config) {}

void CoreSteeringController::computeInternalTargetCourseFrom(
    uint16_t externalTarget) {

  switch (m_navigationSensors.getActiveSource()) {
  case NavigationSource::Compass:
    m_internalTargetCourse = externalTarget;
    break;

  case NavigationSource::Gps:
    /*externelTarget wird über Gps Logik bewertet, damit internalTarget dann COG
    abfangen kann mit gleicher regellogik auf HDG*/
    break;

  case NavigationSource::Wind:
    /* Windwinkel hier verarbeiten*/
    break;

  default:
    m_internalTargetCourse = externalTarget;
  }
}

void CoreSteeringController::setCurrentCourse(uint16_t current) {
  m_currentCourse = current;
}

void CoreSteeringController::tick(unsigned long now) {
  if (now - m_lastImpulse > m_systemConfig.pauseForValidObsAfterImpulse) {
    updateObservationBuffers(now);
  }

  if (now - m_lastImpulse < m_systemConfig.SteeringCooldown_ms)
    return;
  else {
    if (isSteeringCorrectionRequired()) {
      m_pwm.command(calculateSteeringDirectionFromObservation());
    }
  }
}

void CoreSteeringController::updateObservationBuffers(unsigned long now) {
  if (now - m_lastObservation < m_systemConfig.minimumTimeBtwObs)
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
  m_lastObservation = now;

  if (m_observationCount >= OBSERVATION_BUFFER_SIZE) {
    resetObservations();
  }
};

bool CoreSteeringController::isSteeringCorrectionRequired() {
  auto medianArray = m_mergedError;
  std::sort(medianArray.begin(), medianArray.end());

  constexpr size_t kMedianIndex =
      OBSERVATION_BUFFER_SIZE /
      2; // hier nur der upper median bei geraden Mengen von Werten
  m_medianOfMergedErrors = medianArray[kMedianIndex];

  if (abs(m_medianOfMergedErrors) >= m_systemConfig.steeringTolerance_deg) {
    return true;
  }
  return false;
};

SteeringDirection
CoreSteeringController::calculateSteeringDirectionFromObservation() {
  if (m_medianOfMergedErrors <= 0) {
    return SteeringDirection::Left;
  } else if (m_medianOfMergedErrors > 0) {
    return SteeringDirection::Right;
  }
  return SteeringDirection::Left;
};

void CoreSteeringController::resetObservations() {
  m_observationCount = 0;
  for (int i = 0; i <= 99; i++) {
    m_leftErrors[i] = 0;
    m_rightErrors[i] = 0;
    m_mergedError[i] = 0;
  }
};

int16_t CoreSteeringController::calculateHeadingError(uint16_t current,
                                                      uint16_t target) {
  int16_t diff = static_cast<int16_t>(target) - static_cast<int16_t>(current);

  diff += 360;
  diff += 180;
  diff %= 360;
  diff -= 180;

  return diff;
}

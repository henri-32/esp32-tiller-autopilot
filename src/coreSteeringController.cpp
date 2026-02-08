#include "coreSteeringController.h"
#include "globalTypes.h"
#include "navigationSensors.h"
#include "pwmController.h"

#include <Arduino.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>

CoreSteeringController::CoreSteeringController(PWMController &pwm,
                                               NavigationSensors &navsens,
                                               CSC_Config &config)
    : m_pwm(pwm), m_navigationSensors(navsens), m_CSC_Config(config) {}


void CoreSteeringController::tick(unsigned long now) {
  if (observationAllowed(now)) {
    updateObservationBuffers(now);
  }

  if (actionAllowed(now))
    if (steeringCorrectionIsRequired()) {
      m_pwm.command(calculateSteeringDirectionFromObservation());
      m_lastImpulse = now;
      resetObservations();
    }
}

void CoreSteeringController::setCurrentCompassCourse(uint16_t current) {
  m_currentCourse = current;
}

void CoreSteeringController::computeInternalTargetCourseFrom(
    uint16_t externalTarget, NavigationSource callingSource,
    unsigned long now) {

  switch (callingSource) {
  case NavigationSource::Compass:
    // Compass target is intentional and unfiltered.
    // All shaping is handled by CSC itself.
    m_internalTargetCourse = externalTarget;
    break;

  case NavigationSource::Gps:
    /*externelTarget wird über Gps Logik bewertet, damit internalTarget dann COG
    abfangen kann mit gleicher regellogik auf HDG*/
    applyGpsToTargetCourse();
    break;

  case NavigationSource::Wind:
    /* Windwinkel hier verarbeiten*/
    break;

  default:
    m_internalTargetCourse = externalTarget;
  }
}

std::optional<uint16_t>CoreSteeringController::applyGpsToTargetCourse(){
  std::optional<uint16_t> target = std::nullopt;
  if (!GPS_UseAllowed() || m_navigationSensors.sensorValue_valid()) {
  m_navigationSensors.setActiveSource(NavigationSource::Compass); 
  return std::nullopt;
  }

    /* Filterlogik fehlt noch komplett 
    target = filter UND LOGIK auf m_navigationSensors.getCurrentReading();
    */

  return target;
};

bool CoreSteeringController::GPS_UseAllowed(){
  if (m_navigationSensors.getSOG() < m_CSC_Config.minimumGPS_SpeedForGPS_Use) return false; 
  return true;
};
bool CoreSteeringController::observationAllowed(unsigned long now) const {
  if (now - m_lastImpulse > m_CSC_Config.pauseForValidObsAfterImpulse_ms) {
    return true;
  } else
    return false;
};

void CoreSteeringController::updateObservationBuffers(unsigned long now) {
  if (now - m_lastObservation < m_CSC_Config.minimumTimeBtwObs_ms)
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

  if (m_observationCount >= m_CSC_Config.observationBufferSize) {
    resetObservations();
  }
};

bool CoreSteeringController::actionAllowed(unsigned long now) const {
  if (now - m_lastImpulse > m_CSC_Config.SteeringCooldown_ms) {
    return true;
  } else
    return false;
};

bool CoreSteeringController::steeringCorrectionIsRequired() {
  auto medianArray = m_mergedError;
  std::sort(medianArray.begin(), medianArray.end());

  uint8_t medianIndex =
      m_CSC_Config.observationBufferSize /
      2; // hier nur der upper median bei geraden Mengen von Werten
  m_medianOfMergedErrors = medianArray[medianIndex];

  if (abs(m_medianOfMergedErrors) >= m_CSC_Config.steeringTolerance_deg) {
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
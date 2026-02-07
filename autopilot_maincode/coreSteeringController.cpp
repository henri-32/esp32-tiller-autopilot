
#include "coreSteeringController.h"
#include "globalTypes.h"
#include "pwmController.h"
#include <Arduino.h>

CoreSteeringController::CoreSteeringController(PWMController &pwm,
                                               NavigationSensors &navsens)
    : m_pwm(pwm), m_navigationSensors(navsens) {}

void CoreSteeringController::setTargetCourse(uint16_t target) {
  m_courses.targetCourse = target;
}

void CoreSteeringController::setCurrentCourse(uint16_t current) {
  m_courses.currentCourse = current;
}

void CoreSteeringController::computeSteeringDecision() {
  // TimeStamp
  m_steeringDecision.timestamp_ms = millis();

  // Active Source
  m_steeringDecision.m_activeSource = m_navigationSensors.getActiveSource();
  
  // headingError
  m_steeringDecision.headingError_deg =
      getCorrectionInDegrees(static_cast<uint16_t>(m_courses.currentCourse),
                             static_cast<uint16_t>(m_courses.targetCourse));

  // Compute Direction
  if (m_steeringDecision.headingError_deg > 0) {
    m_steeringDecision.steeringDirection = SteeringDirection::Right;
  } else if (m_steeringDecision.headingError_deg < 0) {
    m_steeringDecision.steeringDirection = SteeringDirection::Left;
  }
}

int8_t CoreSteeringController::getCorrectionInDegrees(uint16_t current,
                                                      uint16_t target) {
  int8_t diff = target - current;

  diff += 360; // Sicher positiv
  diff += 180; // Mittelpunkt vorbereiten

  diff %= 360; // Normalisieren
  diff -= 180; // zurück in lage schieben

  return diff;
};

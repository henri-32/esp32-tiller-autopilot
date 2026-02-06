#pragma once
#include "coreSteeringController.h"
#include "pwmController.h"
#include <Arduino.h>

CoreSteeringController::CoreSteeringController(PWMController &pwm)
    : m_pwm(pwm) {}

void CoreSteeringController::setTargetCourse(uint16_t target) {
  m_courses.targetCourse = target;
}

void CoreSteeringController::setCurrentCourse(uint16_t current) {
  m_courses.currentCourse = current;
}

void CoreSteeringController::computeSteeringAction() {
  const int8_t correction =
      getCorrectionInDegrees(static_cast<uint16_t>(m_courses.currentCourse),
                             static_cast<uint16_t>(m_courses.targetCourse));

  if (correction > 0) {
    m_pwm.setImpulse(PWMController::Direction::Right,
                     PWMController::Force::Medium);
  } else if (correction < 0) {
    m_pwm.setImpulse(PWMController::Direction::Left,
                     PWMController::Force::Medium);
  } else {
    m_pwm.setImpulse(PWMController::Direction::Neutral,
                     PWMController::Force::Low);
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

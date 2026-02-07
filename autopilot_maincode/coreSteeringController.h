#pragma once
#include "globalTypes.h"
#include "navigationSensors.h"
#include <Arduino.h>

class PWMController;

class CoreSteeringController {
public:
  struct SteeringDecision {
    unsigned long timestamp_ms;
    NavigationSource m_activeSource;
    int16_t headingError_deg;
    uint16_t tolerance_deg;
    bool inCooldown;

    bool impulseRequested;
    SteeringDirection steeringDirection;
    uint16_t impulse_Duration_ms;
  };

  struct Courses {
    uint16_t targetCourse = 0;
    uint16_t currentCourse = 0;
  };

  explicit CoreSteeringController(PWMController &pwm, NavigationSensors & navsens);

  void setTargetCourse(uint16_t target);
  void setCurrentCourse(uint16_t current);
  void computeSteeringDecision();

private:
  int8_t getCorrectionInDegrees(uint16_t current, uint16_t target);
  PWMController &m_pwm;
  NavigationSensors &m_navigationSensors;
  Courses m_courses{};
  SteeringDecision m_steeringDecision;
};
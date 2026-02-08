#pragma once

#include "globalTypes.h"
#include "navigationSensors.h"
#include <array>
#include <cstdint>

class PWMController;

class CoreSteeringController {
public:
  explicit CoreSteeringController(PWMController &pwm,
                                  NavigationSensors &navsens,
                                  SystemConfig &config);

  void computeInternalTargetCourseFrom(uint16_t target);
  void setCurrentCourse(uint16_t current);

  void tick(unsigned long now);

private:
  PWMController &m_pwm;
  NavigationSensors &m_navigationSensors;
  const SystemConfig &m_systemConfig;
  static constexpr uint8_t OBSERVATION_BUFFER_SIZE = 100;

  int16_t calculateHeadingError(uint16_t current, uint16_t target);
  void updateObservationBuffers(unsigned long now);
  bool isSteeringCorrectionRequired();
  SteeringDirection calculateSteeringDirectionFromObservation();
  void resetObservations();

  uint16_t m_internalTargetCourse = 0;
  uint16_t m_currentCourse = 0;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_leftErrors;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_rightErrors;

  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_mergedError;
  uint8_t m_observationCount = 0;
  int16_t m_medianOfMergedErrors = 0;

  unsigned long m_lastImpulse = 0;
  unsigned long m_lastObservation = 0;
};

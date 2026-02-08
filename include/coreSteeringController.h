#pragma once

#include "globalTypes.h"
#include "navigationSensors.h"
#include <array>
#include <cstdint>
#include <optional>

class PWMController;

class CoreSteeringController {
public:
  explicit CoreSteeringController(PWMController &pwm,
                                  NavigationSensors &navsens,
                                  CSC_Config &config);

  void computeInternalTargetCourseFrom(uint16_t target,
                                       NavigationSource callingSource,
                                       unsigned long now);
  void setCurrentCompassCourse(uint16_t current);

  void tick(unsigned long now);

private:
  PWMController &m_pwm;
  NavigationSensors &m_navigationSensors;
  const CSC_Config &m_CSC_Config;
  static constexpr uint8_t OBSERVATION_BUFFER_SIZE =
      CSC_Config::observationBufferSize;

  uint16_t m_internalTargetCourse = 0;
  uint16_t m_currentCourse = 0;

  int16_t calculateHeadingError(uint16_t current, uint16_t target);
  void updateObservationBuffers(unsigned long now);
  bool steeringCorrectionIsRequired();
  SteeringDirection calculateSteeringDirectionFromObservation();
  void resetObservations();
  bool observationAllowed(unsigned long now) const;
  bool actionAllowed(unsigned long now) const;
  std::optional<uint16_t> applyGpsToTargetCourse();
  bool GPS_UseAllowed();

  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_leftErrors;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_rightErrors;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_mergedError;

  int16_t m_medianOfMergedErrors = 0;

  unsigned long m_lastImpulse = 0;
  unsigned long m_lastObservation = 0;

  uint8_t m_observationCount = 0;
};

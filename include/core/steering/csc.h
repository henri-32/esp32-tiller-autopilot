#pragma once

#include "types/globalTypes.h"
#include <array>
#include <cstdint>
#include <optional>

class PWMController;
class CoreSteeringController {
public:
  explicit CoreSteeringController(SteeringController_Config &config);

  std::optional<SteeringIntent> tick(uint32_t loopTimestamp);
  void currentHDG(uint16_t currentCourse);

private:
  const SteeringController_Config &m_SteeringController_Config;

  static constexpr uint8_t OBSERVATION_BUFFER_SIZE =
      SteeringController_Config::observationBufferSize;

  uint16_t m_internalTargetCourse = 0;
  uint16_t m_currentCourse = 0;

  int16_t normalizeHDGDelta(uint16_t current, uint16_t target);
  void updateObservationBuffers(uint32_t loopTimestamp);
  bool steeringCorrectionIsRequired();
  SteeringDirection calculateSteeringDirectionFromObservation();
  void resetObservations();
  bool observationAllowed(uint32_t loopTimestamp) const;
  bool actionAllowed(uint32_t loopTimestamp) const;

  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_leftErrors;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_rightErrors;
  std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_mergedError;

  int16_t m_medianOfMergedErrors = 0;

  uint32_t m_lastImpulse = 0;
  uint32_t m_lastObservation = 0;

  uint8_t m_observationCount = 0;
};

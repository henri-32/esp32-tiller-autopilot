#include "core/steering/csc.h"
#include "types/globalTypes.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>

CoreSteeringController::CoreSteeringController(
    SteeringController_Config &config)
    : m_SteeringController_Config(config){}

std::optional<SteeringIntent>
CoreSteeringController::tick(uint32_t loopTimestamp) {
  if (observationAllowed(loopTimestamp)) {
    updateObservationBuffers(loopTimestamp);
  }

  SteeringIntent intent;
  intent.abstractImpulse_0_100 =
      m_SteeringController_Config.abstractImpulseDefault;
  intent.dir = calculateSteeringDirectionFromObservation();

  if (!actionAllowed(loopTimestamp)) {
    return std::nullopt;
  }
  if (!steeringCorrectionIsRequired()) {
    return std::nullopt;
  }

  m_lastImpulse = loopTimestamp;
  resetObservations();
  return intent;

}

void CoreSteeringController::currentHDG(uint16_t current) {
  m_currentCourse = current;
}

bool CoreSteeringController::observationAllowed(uint32_t loopTimestamp) const {
  if (loopTimestamp - m_lastImpulse >
      m_SteeringController_Config.pauseForValidObsAfterImpulse_ms) {
    return true;
  } else
    return false;
};

void CoreSteeringController::updateObservationBuffers(uint32_t loopTimestamp) {
  if (loopTimestamp - m_lastObservation <
      m_SteeringController_Config.minimumTimeBtwObs_ms)
    return;

  uint16_t currentError =
      normalizeHDGDelta(m_currentCourse, m_internalTargetCourse);
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

bool CoreSteeringController::actionAllowed(uint32_t loopTimestamp) const {
  if (loopTimestamp - m_lastImpulse >
      m_SteeringController_Config.SteeringCooldown_ms) {
    return true;
  } else
    return false;
};

bool CoreSteeringController::steeringCorrectionIsRequired() {
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
  for (int i = 0; i < OBSERVATION_BUFFER_SIZE; i++) {
    m_leftErrors[i] = 0;
    m_rightErrors[i] = 0;
    m_mergedError[i] = 0;
  }
};

int16_t CoreSteeringController::normalizeHDGDelta(uint16_t current,
                                                  uint16_t target) {
  int16_t diff = static_cast<int16_t>(target) - static_cast<int16_t>(current);

  diff += 360;
  diff += 180;
  diff %= 360;
  diff -= 180;

  return diff;
}

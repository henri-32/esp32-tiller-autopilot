#include "core/steering/csc/csc.h"

#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/deadband.h"
#include "core/steering/csc/actionGuard.h"
#include <cstdint>

CoreSteeringController::CoreSteeringController(
    SteeringController_Config& config)
    : m_config(config),
      m_observationBuffer(nullptr),
      m_deadband(nullptr),
      m_actionGuard(nullptr) {}

std::optional<SteeringIntent>
CoreSteeringController::tick(uint32_t loopTimestamp) {
    return std::nullopt;
}

void CoreSteeringController::currentHDG(uint16_t current) {
  m_currentCourse = current;
}

void CoreSteeringController::setInternalTarget(uint16_t target) {
  m_internalTargetCourse = target;
};

uint16_t CoreSteeringController::getInternalTarget() const{
  return m_internalTargetCourse;
};

SteeringDirection
CoreSteeringController::determineDirection(int16_t median) const {
    return SteeringDirection::Left;
}

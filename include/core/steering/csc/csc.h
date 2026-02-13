#pragma once

#include "core/config.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <optional>

// Forward Declarations der Submodule
class HeadingErrorCalculator;
class ObservationBuffer;
class Deadband;
class ActionGuard;

class CoreSteeringController {
public:
    explicit CoreSteeringController(SteeringController_Config& config);

    std::optional<SteeringIntent> tick(uint32_t loopTimestamp);

    void currentHDG(uint16_t current);
    void setInternalTarget(uint16_t target);
    uint16_t getInternalTarget() const;

private:
    // --- Configuration ---
    SteeringController_Config& m_config;

    // --- State ---
    uint16_t m_currentCourse{0};
    uint16_t m_internalTargetCourse{0};

    // --- Subsystems ---
    HeadingErrorCalculator* m_errorCalculator;
    ObservationBuffer* m_observationBuffer;
    Deadband* m_deadband;
    ActionGuard* m_actionGuard;

    // --- Internal helpers ---
    SteeringDirection determineDirection(int16_t median) const;
};

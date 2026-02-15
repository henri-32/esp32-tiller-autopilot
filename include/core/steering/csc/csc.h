#pragma once

#include "core/config.h"
#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <optional>
  struct CSCDebugSnapshot {
    int16_t error = 0;
    int16_t median = 0;
    uint8_t sampleSize = 0;
    bool deadbandActive = false;
    bool observationBlocked = false;
    bool intentBlocked = false;
  };

class CoreSteeringController {
public:
  // Contract:
  // Purpose: Decide if a steering impulse should be issued.
  // Inputs: current heading, internal target, regulation config slice.
  // Outputs/Side-effects: optional SteeringIntent; updates internal state.
  explicit CoreSteeringController(SteeringRegulationConfig &config);

  std::optional<SteeringIntent> tick(uint32_t loopTimestamp);

  void currentHDG(uint16_t current);
  void setInternalTarget(uint16_t target);
  uint16_t getInternalTarget() const;
  const CSCDebugSnapshot& getDebug() const;


private:
  // --- State ---
  uint16_t m_currentCourse{0};
  uint16_t m_internalTargetCourse{0};

  // --- Subsystems ---
  HeadingErrorCalculator m_errorCalculator;
  ObservationBuffer m_observationBuffer;
  Deadband m_deadband;
  SteeringGuard m_steeringGuard;

  // --- Internal helpers ---
  // Direction is derived from the median error sign only.
  SteeringDirection determineDirection(int16_t median) const;
  uint32_t m_lastObsUpdate = 0;
  uint32_t m_lastIntent = 0;

  // --- Debug ---
  CSCDebugSnapshot m_debug;

};

#pragma once

#include "config/config.h"
#include "config/config.h"
#include "core/queueServer.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include "types/steeringTypes.h"
#include <cstdint>
#include <optional>

struct CscLogValues
{
  int16_t hdg_error = 0;
  int16_t hdg_median = 0;
  uint8_t sampleSize = 0;
  bool observationBlocked = false;
  bool intentBlocked = false;
};

class CoreSteeringController
{
public:
  // Contract:
  // Purpose: Decide if a steering impulse should be issued.
  // Inputs: current heading, internal target, regulation config slice.
  // Outputs/Side-effects: optional SteeringIntent; updates internal state.
  explicit CoreSteeringController(QueueServer* qServer);

  void currentHDG(uint16_t current);
  void setInternalTarget(uint16_t target);
  uint16_t getInternalTarget() const;
  const CscLogValues& getDebug() const;
  SteeringIntent tick(uint32_t loopTimestamp);

private:
  // --- Internal helpers ---
  // Direction is derived from the median error sign only.
  SteeringDirection determineDirection(int16_t median) const;
  SteeringIntent calculateIntentFromObs();
  SteeringIntent counterIntent(float omega);
  bool counterIntentNecessary(int16_t median, uint8_t sampleSize, float omega,
                              uint32_t loopTimestamp);

  // --- State ---
  uint16_t m_currentCourse{0};
  uint16_t m_internalTargetCourse{0};
  int16_t m_lastError = 0;
  uint32_t m_lastObsUpdate = 0;
  uint32_t m_lastIntent = 0;
  uint32_t m_lastCounterIntent = 0;
  std::optional<SteeringIntent> m_lastIntentValue;

  // --- Subsystems ---
  HeadingErrorCalculator m_errorCalculator;
  ObservationBuffer m_observationBuffer;
  SteeringGuard m_steeringGuard;
  const SteeringRegulationConfig m_config{};

  // --- Debug ---
  CscLogValues m_log;
};

#pragma once
#include "core/config.h"
#include "core/steering/sourceModels/gpsModel.h"
#include "core/steering/sourceModels/windModel.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include <cstdint>

class SourcePolicyEngine {
public:
  // Contract:
  // Purpose: Select the effective navigation source with fallbacks.
  // Inputs: requested source + sensor snapshot + policy config slice.
  // Outputs/Side-effects: effective source; may emit diagnostics.
  SourcePolicyEngine(const SteeringSourceHandlingConfig &config,
                     Diagnostics &Diagnostics);

  NavigationSource evaluate(NavigationSource requested,
                            const NavigationSensors::NavigationSnapshot &snapshot,
                            uint32_t loopTimestamp);

private:
  const SteeringSourceHandlingConfig &m_config;
  Diagnostics &m_diagnostics;
};

class SourceTargetInterpreter {
public:
  // Contract:
  // Purpose: Filter a target based on the effective source.
  // Inputs: general target + effective source + source models.
  // Outputs/Side-effects: filtered target; no side-effects.
  SourceTargetInterpreter() = default;
  uint16_t applySourceFilters(uint16_t generalTarget,
                              NavigationSource effective);

private:
  GPSModel m_gpsModel;
  WindModel m_windModel;
};

class SourceExecutor {

public:
  // Contract:
  // Purpose: Apply effective source
  // Inputs: effective source.
  // Outputs/Side-effects: updates NavigationSensors
  SourceExecutor(NavigationSensors &navsens);

  void execute(NavigationSource effective);

private:
  NavigationSensors &m_navigationSensors;
};

class SourceHandler {
public:
  // Contract:
  // Purpose: Orchestrate policy, filtering, and execution for sources.
  // Inputs: snapshot, requested source, target, and config slice.
  // Outputs/Side-effects: returns filtered CSC target; updates lead source and
  // diagnostics.
  explicit SourceHandler(NavigationSensors &navsens,
                         const SteeringSourceHandlingConfig &config,
                         Diagnostics &diagnostics);

  // TODO(Architektur):
  // Mittelfristig statt nur uint16_t einen Guidance-Output liefern, z. B.:
  // - effectiveSource
  // - currentHDG (bereits validiert)
  // - internalTargetHDG
  // - valid/blocked-Flag
  // Damit bleibt die Verantwortung "Kursableitung + Validierung" in einer
  // Schicht und der Orchestrator muss keine Sensorfelder direkt kennen.
  uint16_t tick(const NavigationSensors::NavigationSnapshot &snapshot,
                NavigationSource requestedSource, uint16_t generalTarget,
                uint32_t loopTimestamp);

private:
  SourcePolicyEngine m_sourcePolicy;
  SourceTargetInterpreter m_targetInterpreter;
  SourceExecutor m_sourceExecutor;
};

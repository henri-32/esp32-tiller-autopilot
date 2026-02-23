#pragma once
#include "core/config.h"
#include "core/steering/sourceModels/gpsModel.h"
#include "core/steering/sourceModels/windModel.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "ui/controlPanel.h"
#include <array>
#include <cstdint>


class SourcePolicyEngine {
public:
  // Contract:
  // Purpose: Select the effective navigation source with fallbacks.
  // Inputs: requested source + sensor snapshot + policy config slice.
  // Outputs/Side-effects: effective source; may emit diagnostics.
  SourcePolicyEngine(SteeringSourceHandlingConfig &config,
                     Diagnostics &diagnostics);

  NavigationSource evaluate(NavigationSource requested,
                            NavigationSensors::NavigationSnapshot snapshot,
                            uint32_t loopTimestamp);

private:
  SteeringSourceHandlingConfig &m_config;
  Diagnostics &m_diagnostics;
};

class SourceTargetInterpreter {
public:
  // Contract:
  // Purpose: Filter a target based on the effective source.
  // Inputs: general target + effective source + source models.
  // Outputs/Side-effects: filtered target; no side-effects.
  SourceTargetInterpreter(GPSModel &gpsModel, WindModel &windmodel);
  uint16_t applySourceFilters(uint16_t generalTarget,
                              NavigationSource effective);

private:
  GPSModel &m_gpsModel;
  WindModel &m_windModel;
};

class SourceExecutor {

public:
  // Contract:
  // Purpose: Apply effective source 
  // Inputs: effective source + filtered target.
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
  // Inputs: requested source, snapshot, target, and config slice.
  // Outputs/Side-effects: updates CSC target, navigation lead source, diagnostics.
  explicit SourceHandler(const ControlPanel &panel, NavigationSensors &navsens,
                        SteeringSourceHandlingConfig &config,
                         Diagnostics &diagnostics);

 uint16_t tick(NavigationSource requestedSource, uint32_t loopTimestamp,
            NavigationSensors::NavigationSnapshot snapshot,
            uint16_t generalTarget);

private:
  const ControlPanel &m_panel;
  NavigationSensors &m_navigationSensors;
  Diagnostics &m_diagnostics;

  GPSModel m_gpsModel;
  WindModel m_windModel;
  SourcePolicyEngine m_sourcePolicy;
  SourceTargetInterpreter m_targetInterpreter;
  SourceExecutor m_sourceExecutor;

  static constexpr uint8_t OBSERVATION_BUFFER_SIZE =
      SteeringSourceHandlingConfig::observationBufferSize;
  std::array<uint16_t, OBSERVATION_BUFFER_SIZE> m_observationBuffer;
};

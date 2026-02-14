#pragma once
#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "core/steering/sourceModels/gpsModel.h"
#include "core/steering/sourceModels/windModel.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "ui/controlPanel.h"
#include <array>
#include <cstdint>

class SourcePolicyEngine {
public:
  SourcePolicyEngine(SteeringController_Config &config,
                     Diagnostics &diagnostics);

  NavigationSource evaluate(NavigationSource requested,
                            NavigationSensors::NavigationSnapshot snapshot,
                            uint32_t loopTimestamp);

private:
  SteeringController_Config &m_config;
  Diagnostics &m_diagnostics;
};

class SourceTargetInterpreter {
public:
  SourceTargetInterpreter(GPSModel& gpsModel, WindModel& windmodel);
  uint16_t applySourceFilters(uint16_t generalTarget, NavigationSource effective);

private:
  GPSModel& m_gpsModel; 
  WindModel& m_windModel; 
};

class SourceExecutor {

  public:
  SourceExecutor(NavigationSensors& navsens, CoreSteeringController& csc);

  void execute(NavigationSource effective, uint16_t filteredTarget);

  private:
  NavigationSensors& m_navigationSensors; 
  CoreSteeringController& m_csc; 
};

class SourceHandler {
public:
  explicit SourceHandler(const ControlPanel &panel, NavigationSensors &navsens,
                         SteeringController_Config &config,
                         Diagnostics &diagnostics, CoreSteeringController &csc);

  void tick(NavigationSource requestedSource, uint32_t loopTimestamp,
            NavigationSensors::NavigationSnapshot snapshot, uint16_t generalTarget);

private:
  const ControlPanel &m_panel;
  NavigationSensors &m_navigationSensors;
  SteeringController_Config &m_steeringController_Config;
  Diagnostics &m_diagnostics;
  CoreSteeringController &m_csc;

  GPSModel m_gpsModel;
  WindModel m_windModel;
  SourcePolicyEngine m_sourcePolicy;
  SourceTargetInterpreter m_targetInterpreter;
  SourceExecutor m_sourceExecutor;

  static constexpr uint8_t OBSERVATION_BUFFER_SIZE =
      SteeringSourceEVConfig::observationBufferSize;
  std::array<uint16_t, OBSERVATION_BUFFER_SIZE> m_observationBuffer;
};

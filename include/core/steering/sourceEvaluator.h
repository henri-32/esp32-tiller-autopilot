#pragma once
#include "core/config.h"
#include "core/steering/csc.h"
#include "sensors/navigationSensors.h"
#include "diagnostics/diagnostics.h"
#include "ui/controlPanel.h"
#include <array>
#include <cstdint>

class SourceEvaluator {
public:
  explicit SourceEvaluator(const ControlPanel& panel, NavigationSensors &navsens,
                           SteeringController_Config &config,
                           Diagnostics &diagnostics);

  void tick(NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc,
            uint32_t loopTimestamp);

  void evaluateGPS( NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc, uint32_t loopTimestamp);
  void evaluateCompass(NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc, uint32_t loopTimestamp);
  void evaluateWind (NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc, uint32_t loopTimestamp);
private:
  const ControlPanel & m_panel; 
  NavigationSensors &m_navigationSensors;
  SteeringController_Config &m_steeringController_Config;
  Diagnostics &m_diagnostics;
  static constexpr uint8_t OBSERVATION_BUFFER_SIZE = SteeringSourceEVConfig::observationBufferSize;
  std::array<uint16_t, OBSERVATION_BUFFER_SIZE> m_observationBuffer;
  uint32_t m_lastObservation; 
  uint32_t m_lastCorrection;
  uint16_t m_observationCounter; 
};

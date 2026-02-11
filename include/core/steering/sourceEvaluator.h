#pragma once
#include "core/config.h"
#include "core/steering/csc.h"
#include "sensors/navigationSensors.h"
#include "diagnostics/diagnostics.h"
#include <cstdint>
class SourceEvaluator {
public:
  explicit SourceEvaluator(NavigationSensors &navsens,
                           SteeringController_Config &config,
                           Diagnostics &diagnostics);

  void tick(NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc,
            uint32_t loopTimestamp);

  void evaluateGPS(NavigationSensors::NavigationSnapshot snapshot, uint32_t loopTimestamp);
  void evaluateCompass(NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController& csc, uint32_t loopTimestamp);
  
private:
  NavigationSensors &m_navigationSensors;
  SteeringController_Config &m_steeringController_Config;
  Diagnostics &m_diagnostics;
};

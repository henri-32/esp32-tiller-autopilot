#pragma once
#include "sensors/navigationSensors.h"
#include "diagnostics/diagnostics.h"
  class SourceEvaluator {
  public:
    explicit SourceEvaluator(NavigationSensors &navsens,
                             SteeringController_Config &config,
                             Diagnostics &diagnostics);

    void tick(NavigationSensors::NavigationSnapshot snapshot,
              uint32_t loopTimestamp);

  private:
    NavigationSensors &m_navigationSensors;
    SteeringController_Config &m_steeringController_Config;
    Diagnostics &m_diagnostics;
  };
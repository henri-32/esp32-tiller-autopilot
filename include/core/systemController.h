#pragma once
#include "actuators/pwmController.h"
#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "core/steering/impulseFilter.h"
#include "core/steering/sourceHandling.h"
#include "core/steering/steeringOrchestrator.h"
#include "diagnostics/diagnostics.h"
#include "sensors/NMEA183BUS.h"
#include "sensors/navigationSensors.h"
#include "ui/controlPanel.h"
#include "ui/display.h"

class SystemController {
public:
  // Contract:
  // Purpose: Compose modules and run the top-level control loop.
  // Inputs: loop timestamp; internal sensors and UI state.
  // Outputs/Side-effects: drives actuators, diagnostics, and display updates.
  SystemController();

  void tick(uint32_t loopTimestamp);

private:
  // Top-level composition root. Owns hardware modules, config, and orchestration.
  ControlPanel m_controlPanel{};
  CompassModule m_compassModule{};
  GPSModule m_gpsModule;
  WindModule m_windModule;
  NMEA183BUS m_nmea183Bus;
  NavigationSensors m_navigationSensors;
  PWMController m_pwmController{};
  SteeringControllerConfig m_config;
  Diagnostics m_diagnostics;
  ImpulseFilter m_impulseFilter;
  CoreSteeringController m_csc;
  SourceHandler m_sourceHandler;
  SteeringOrchestrator m_steeringOrchestrator;
  Display m_display;
};

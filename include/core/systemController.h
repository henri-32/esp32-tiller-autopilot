#pragma once
#include "actuators/pwmController.h"
#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "sensors/NMEA183BUS.h"
#include "sensors/navigationSensors.h"
#include "core/steering/impulseFilter.h"
#include "core/steering/sourceEvaluator.h"
#include "core/steering/steeringOrchestrator.h"
#include "ui/controlPanel.h"
#include "ui/display.h"
#include "diagnostics/diagnostics.h"

class SystemController {
public:
  SystemController();

  void tick(uint32_t loopTimestamp);

private:
  ControlPanel m_controlPanel{};
  CompassModule m_compassModule{};
  GPSModule m_gpsModule;
  WindModule m_windModule;
  NMEA183BUS m_nmea183Bus;
  NavigationSensors m_navigationSensors;
  PWMController m_pwmController{};
  SteeringController_Config m_SteeringController_Config;
  Diagnostics m_diagnostics;
  ImpulseFilter m_impulseFilter;
  CoreSteeringController m_csc;
  SourceEvaluator m_sourceEvaluator;
  SteeringOrchestrator m_steeringOrchestrator;
  Display m_display;
};


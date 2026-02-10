#pragma once
#include "actuators/pwmController.h"
#include "core/steeringController.h"
#include "sensors/NMEA183BUS.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include "ui/controlPanel.h"
#include "ui/display.h"
#include "diagnostics/diagnostics.h"

class SystemController {
public:
  SystemController();

  void tick(unsigned long loopTimestamp);

private:
  ControlPanel m_controlPanel{};
  CompassModule m_compassModule{};
  GPSModule m_gpsModule;
  WindModule m_windModule;
  NMEA183BUS m_nmea183Bus;
  NavigationSensors m_navigationSensors;
  PWMController m_pwmController{};
  SteeringController m_steeringController;
  Diagnostics m_diagnostics;
  Display m_display;
  SteeringController_Config m_SteeringController_Config;
};

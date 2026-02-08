#pragma once 
#include "display.h"
#include "globalTypes.h"
#include "navigationSensors.h"
#include "pwmController.h"
#include "controlPanel.h"
#include "coreSteeringController.h"
#include "NMEA183BUS.h"

class SystemController {
    public: 
    SystemController();

    void tick();

    private:

    ControlPanel m_controlPanel{};
    CompassModule m_compassModule{};
    GPSModule m_gpsModule;
    WindModule m_windModule;
    NMEA183BUS m_nmea183Bus;
    NavigationSensors m_navigationSensors;
    PWMController m_pwmController{};
    CoreSteeringController m_coreSteeringController;
    Display m_display;
    CSC_Config m_CSC_Config;

};

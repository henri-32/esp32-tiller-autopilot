#pragma once 
#include "display.h"
#include "navigationSensors.h"
#include "pwmController.h"
#include "controlPanel.h"
#include "coreSteeringController.h"

class SystemController {
    public: 
    SystemController();

    void tick();

    private:

    ControlPanel m_controlPanel{};
    CompassModule m_compassModule{};
    GPSModule m_gpsModule;
    WindModule m_windModule;
    NavigationSensors m_navigationSensors;
    PWMController m_pwmController{};
    CoreSteeringController m_coreSteeringController;
    Display m_display;
    SystemConfig m_systemConfig;

};
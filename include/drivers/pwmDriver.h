#pragma once
#include "config/config.h"
#include "types/steeringTypes.h"
#include "esp_err.h"


class PwmDriver {
public:
  // Contract:
  // Purpose: Drive the steering actuator from a filtered PWM intent.
  // Inputs: PWMIntent with direction + filtered abstract impulse + duration.
  // Outputs/Side-effects: updates actuator state (hardware output).

  PwmDriver() {};


  void command(const PWMIntent &cmd);

  esp_err_t init();

private:
  SteeringDirection m_lastDir;

  //Defines the LEDC_CHANNELS for the Steering Direction 
  uint8_t starbordChannel_ ;
  uint8_t portsideChannel_ ;
  
  //Last used dutyCycle to make sure only one channel is high
  uint8_t starbordDuty_ = 0;
  uint8_t portsideDuty_ = 0;
};

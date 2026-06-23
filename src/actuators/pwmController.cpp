#include "actuators/pwmController.h"
#include "types/steeringTypes.h"

PWMController::PWMController(const SteeringMechanicsConfig &config)
    : m_config(config) {};

void PWMController::command(const PWMIntent &command) {
  // TODO HardwareImpulse ausgeben
  //Auskommentiert um clangd Warnungen zu ignorieren
  
  //const auto dir = command.dir;

  /*uint32_t pulse_ms =
      m_config.steeringMinImpulse_ms +
      (command.filteredAbstractImpulse_0_100 / 100.0f) *
          (m_config.steeringMaxImpulse_ms - m_config.steeringMinImpulse_ms);*/
}

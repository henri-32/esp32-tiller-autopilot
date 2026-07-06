#include "drivers/pwmDriver.h"
#include "types/steeringTypes.h"


void PwmDriver::command(const PWMIntent &command) {
  // TODO HardwareImpulse ausgeben
  
  const auto dir = command.dir;

  uint32_t pulse_ms =
      m_config.steeringMinImpulse_ms +
      (command.filteredAbstractImpulse_0_100 / 100.0f) *
          (m_config.steeringMaxImpulse_ms - m_config.steeringMinImpulse_ms);
}

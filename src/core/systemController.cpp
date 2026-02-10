#include "core/systemController.h"
#include "sensors/navigationSensors.h"

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      m_steeringController(m_pwmController, m_navigationSensors,
                           m_SteeringController_Config, m_diagnostics) {}

void SystemController::tick(unsigned long loopTimestamp) {

  // 1. Intent lesen
  const auto intent = m_controlPanel.readIntent();

  // 2. Quelle wählen und Ziel setzen
  m_navigationSensors.setActiveSource(m_controlPanel.m_activeSource);

  // 3. Datenfluss von Istwert zur Regelung
  const auto current = m_navigationSensors.getCurrentReading();

  m_steeringController.setCurrentCompassCourse(current.value());

  // 4. Logik ausführen und Hardware betätigen
  m_steeringController.tick(loopTimestamp);

  // 5. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 6. Display updaten
  m_display.update();
}

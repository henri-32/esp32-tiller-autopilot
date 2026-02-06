#include "systemController.h"

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule),
      m_coreSteeringController(m_pwmController) {}

void SystemController::tick() {
  // 1. Intent lesen
  const auto intent = m_controlPanel.readIntent();

  // 2. Quelle wählen und Ziel setzen
  m_navigationSensors.setActiveSource(mapSource(intent.source));
  m_coreSteeringController.setTargetCourse(intent.targetCourse);

  // 3. Datenfluss von Istwert zur Regelung
  const uint16_t current = m_navigationSensors.getCurrentReading();
  m_coreSteeringController.setCurrentCourse(current);

  // 4. Logik ausführen und Hardware betätigen
  m_coreSteeringController.computeSteeringAction();

  // 5. Display updaten
  m_display.update();
}

NavigationSensors::ActiveSource
SystemController::mapSource(ControlPanel::Source src) {
  switch (src) {
  case ControlPanel::Source::Compass:
    return NavigationSensors::ActiveSource::Compass;

  case ControlPanel::Source::Gps:
    return NavigationSensors::ActiveSource::Gps;

  case ControlPanel::Source::Wind:
    return NavigationSensors::ActiveSource::Wind;

  default:
    return NavigationSensors::ActiveSource::Compass;
  }
}
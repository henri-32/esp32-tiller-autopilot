#include "systemController.h"
#include "NMEA183BUS.h"
#include <Arduino.h>

SystemController::SystemController()
    : m_nmea183Bus(), m_navigationSensors(m_compassModule, m_gpsModule,
                                          m_windModule, m_nmea183Bus),
      m_coreSteeringController(m_pwmController, m_navigationSensors,
                               m_CSC_Config) {}

void SystemController::tick() {
  unsigned long now = millis();
  // 1. Intent lesen
  const auto intent = m_controlPanel.readIntent();

  // 2. Quelle wählen und Ziel setzen
  m_navigationSensors.setActiveSource(m_controlPanel.m_activeSource);
  m_coreSteeringController.computeInternalTargetCourseFrom(
      intent.targetCourse, m_navigationSensors.getActiveSource(), now);

  // 3. Datenfluss von Istwert zur Regelung
  const auto current = m_navigationSensors.getCurrentReading();

  m_coreSteeringController.setCurrentCompassCourse(current.value());

  // 4. Logik ausführen und Hardware betätigen
  m_coreSteeringController.tick(millis());

  // 5. Display updaten
  m_display.update();
}

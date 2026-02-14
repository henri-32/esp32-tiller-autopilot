#include "core/systemController.h"
#include "sensors/navigationSensors.h"
#include "ui/controlPanelTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      m_impulseFilter(m_SteeringController_Config),
      m_csc(m_SteeringController_Config),
      m_sourceHandler(m_controlPanel, m_navigationSensors,
                      m_SteeringController_Config, m_diagnostics, m_csc),
      m_steeringOrchestrator( m_csc, m_impulseFilter,
                             m_pwmController) {}

void SystemController::tick(uint32_t loopTimestamp) {
  // Überblick übers System
  const auto snapshot = m_navigationSensors.createSnapshot();
  const auto intent = m_controlPanel.readIntent();

  // 2. Quelle waehlen
  // Verarbeitet aktive Sensorquelle und Fallbacks der Sensoren
  m_sourceHandler.tick(intent.requestedSource, loopTimestamp, snapshot,
                       intent.generalTarget);

  // 3. Steering Ausführen
  if (intent.steeringEngaged) {
    m_steeringOrchestrator.tick(snapshot, loopTimestamp);
  }

  // 4. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 5. Display updaten
  m_display.update();
}

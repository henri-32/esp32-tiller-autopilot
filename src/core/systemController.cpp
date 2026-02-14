#include "core/systemController.h"
#include "sensors/navigationSensors.h"
#include "ui/controlPanelTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      // Config is owned here and passed down in granular slices.
      m_impulseFilter(m_config.mechanics, m_config.physics),
      m_csc(m_config.regulations),
      m_sourceHandler(m_controlPanel, m_navigationSensors,
                      m_config.source, m_diagnostics, m_csc),
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
  // Only drive the actuator path when steering is explicitly engaged.
  if (intent.steeringEngaged) {
    m_steeringOrchestrator.tick(snapshot, loopTimestamp);
  }

  // 4. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 5. Display updaten
  m_display.update();
}

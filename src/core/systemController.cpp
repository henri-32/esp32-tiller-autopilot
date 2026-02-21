#include "core/systemController.h"

#include "types/globalTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      // Config is owned here and passed down in granular slices.
      m_impulseFilter(m_config.mechanics, m_config.physics),
      m_csc(m_config.regulations),
      m_sourceHandler(m_controlPanel, m_navigationSensors, m_config.source,
                      m_diagnostics, m_csc),
      m_steeringOrchestrator(m_csc, m_impulseFilter, m_pwmController),
      m_displayContent( m_navigationSensors, m_diagnostics,
                       m_state) {}

void SystemController::tick(uint32_t loopTimestamp) {
  // Überblick übers System
  const auto snapshot = m_navigationSensors.createSnapshot();
  const auto intent = m_controlPanel.readIntent();

  // 2. Quelle waehlen
  // Verarbeitet aktive Sensorquelle und Fallbacks der Sensoren
  m_sourceHandler.tick(intent.activeSource, loopTimestamp, snapshot,
                       intent.generalTarget);

  // 3. Steering Ausführen
  // Only drive the actuator path when steering is explicitly engaged.
  if (intent.steeringEngaged &&
      m_state.systemMode == SystemState::SystemMode::OK) {
    m_steeringOrchestrator.tick(snapshot, loopTimestamp);
  }

  // 4. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 6. Systemstatus überprüfen
  stateUpdate();

  // 7. Display updaten
  auto content =  m_displayContent.tick(intent, loopTimestamp);
  m_display.update(content);
}

void SystemController::stateUpdate() {
  // 1. Error Handling
  if (m_diagnostics.criticalErroroccured) {
    m_state.systemMode = SystemState::SystemMode::SAFE;
  };
}
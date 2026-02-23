#include "core/systemController.h"

#include "types/globalTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(),
    m_sourceHandler(m_controlPanel, m_navigationSensors, m_config.source, m_diagnostics),
      m_steeringOrchestrator(m_config),
      m_displayContent(m_navigationSensors, m_diagnostics) {}

void SystemController::tick(uint32_t loopTimestamp) {
  // Überblick übers System
  const auto nav_snapshot = m_navigationSensors.createSnapshot();
  const auto panel_intent = m_controlPanel.readIntent();

  // 2. Quelle waehlen
  // Verarbeitet aktive Sensorquelle und Fallbacks der Sensoren
  auto cscTarget = m_sourceHandler.tick(panel_intent.activeSource, loopTimestamp, nav_snapshot,
                       panel_intent.generalTarget);

  // 3. Steering Ausführen
  // Only drive the actuator path when steering is explicitly engaged.
  if (panel_intent.steeringEngaged &&
      m_state.systemMode == SystemState::SystemMode::OK) {
    m_steeringOrchestrator.tick(nav_snapshot, cscTarget, loopTimestamp);
  }

  // 4. Diagnostics
  //Gerade noch getrennt, weil tick wahrscheinlich mehr machen wird als den snapshot
  m_diagnostics.tick(loopTimestamp);
  const auto diagnostics_snapshot = m_diagnostics.snapshot(); 

  // 6. Systemstatus überprüfen
  m_state = stateUpdate(diagnostics_snapshot);

  // 7. Display updaten
  auto content = m_displayContent.create(panel_intent, m_state, loopTimestamp);
  m_display.update(content);
}

SystemState SystemController::stateUpdate(DiagnosticSnapshot snapshot) {
  SystemState state;
  // 1. Error Handling
  if (snapshot.criticalErrorOccured) {
    state.systemMode = SystemState::SystemMode::SAFE;
  };

  return state;
}
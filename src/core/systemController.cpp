#include "core/systemController.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include "ui/controlPanelTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      m_impulseFilter(m_SteeringController_Config),
      m_csc(m_SteeringController_Config),
      m_sourceEvaluator(m_controlPanel, m_navigationSensors,
                        m_SteeringController_Config, m_diagnostics),
      m_steeringOrchestrator(m_sourceEvaluator, m_csc, m_impulseFilter,
                             m_pwmController) {}

void SystemController::tick(uint32_t loopTimestamp) {

  // 1. Intent lesen
  PanelIntent loopIntent = m_controlPanel.readIntent();

  // 2. Quelle waehlen
  m_navigationSensors.setLeadSource(
      loopIntent.activeSource); // TODO Bei Event direkt vom panel to
                                // sensors.Außerdem überschreibt gerade Panel
                                // jede Loop die Fallbacks !!!!!

  // 3. Datenfluss
  const auto snapshot = m_navigationSensors.createSnapshot();

  // if (!snapshot.compass_hdg_dg.valid) { CRITICALERROR}; // deswegen soll es
  // nicht zwischen Modulen passieren
  if (snapshot.compass_hdg_dg.valid) {
    m_csc.currentHDG(snapshot.compass_hdg_dg.value);
  };

  // 4. Steering Ausführen
  m_steeringOrchestrator.tick(snapshot, loopTimestamp);

  // 5. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 6. Display updaten
  m_display.update();
}

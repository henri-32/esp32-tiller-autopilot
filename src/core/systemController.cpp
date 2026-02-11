#include "core/systemController.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(m_compassModule, m_gpsModule, m_windModule,
                          m_nmea183Bus),
      m_impulseFilter(m_SteeringController_Config),
      m_csc(m_SteeringController_Config),
      m_sourceEvaluator(m_navigationSensors, m_SteeringController_Config,
                        m_diagnostics),
      m_steeringOrchestrator(m_sourceEvaluator, m_csc, m_impulseFilter,
                             m_pwmController) {}

void SystemController::tick(uint32_t loopTimestamp) {

  // 1. Intent lesen
  m_controlPanel.readIntent();

  // 2. Quelle waehlen und Ziel setzen
  m_navigationSensors.setLeadSource(
      m_controlPanel.m_activeSource); // TODO direkt zwischen Modulen

  // 3. Datenfluss von Istwert zur Regelung
  const auto snapshot = m_navigationSensors.createSnapshot();

  // if (!snapshot.compass_hdg_dg.valid) { CRITICALERROR};
  if (snapshot.compass_hdg_dg.valid) {
    m_csc.currentHDG(snapshot.compass_hdg_dg.value);
  };

  // 4. Logik ausfÃ¼hren und Hardware betÃ¤tigen
  m_steeringOrchestrator.tick(snapshot, loopTimestamp);

  // 5. Diagnostics
  m_diagnostics.tick(loopTimestamp);

  // 6. Display updaten
  m_display.update();
}

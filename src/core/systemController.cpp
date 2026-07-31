#if 0
// Legacy synchronous architecture. Kept as reference until its remaining
// ideas have been migrated to the FreeRTOS-based implementation.
#include "core/systemController.h"
#include "types/controllerTypes.h"
#include "types/sensorTypes.h"
#include "types/steeringTypes.h"
#include <cstdint>

SystemController::SystemController()
    : m_navigationSensors(), m_sourceHandler(m_navigationSensors, m_config.source, m_diagnostics),
      m_steeringOrchestrator(m_config)
{
}

void SystemController::tick(uint32_t loopTimestamp)
{
    // 1. Überblick übers System
    const auto nav_snapshot = m_navigationSensors.createSnapshot();
    const auto panel_intent = m_controlPanel.readIntent();

    // 2. Quelle waehlen
    // Verarbeitet aktive Sensorquelle und Fallbacks der Sensoren
    auto cscTarget = m_sourceHandler.tick(nav_snapshot, panel_intent.activeSource,
                                          panel_intent.generalTarget, loopTimestamp);

    // 3. Steering Ausführen
    // Only drive the actuator path when steering is explicitly engaged.
    if (panel_intent.steeringEngaged && m_state.systemMode == SystemState::SystemMode::OK)
    {
        m_steeringOrchestrator.tick(nav_snapshot, cscTarget, loopTimestamp);
    }

    // TODO(Architektur):
    // State-Update als explizite Transition modellieren:
    // nextState = f(previousState, diagnostics, panelIntent)
    // Dann bleiben Mode-Wechsel deterministisch und "INIT/OK/SAFE"-Regeln sind
    // zentral an einer Stelle dokumentiert.

    // 4. Diagnostics
    // Gerade noch getrennt, weil tick wahrscheinlich mehr machen wird als den
    // snapshot
    //
    m_diagnostics.tick(loopTimestamp);
    const auto diagnostics_snapshot = m_diagnostics.snapshot();

    // 5.. Systemstatus überprüfen
    m_state = stateUpdate(diagnostics_snapshot);

    // 6. Display updaten
    auto content = m_displayContent.renderBuffer(panel_intent, nav_snapshot, diagnostics_snapshot,
                                           m_state.systemMode, m_config.display, loopTimestamp);
    m_display.update();
}

SystemState SystemController::stateUpdate(const DiagnosticSnapshot& snapshot)
{
    // TODO(Architektur):
    // Diese Funktion sollte den bisherigen Zustand beruecksichtigen, statt immer
    // von einem neu erzeugten Default-State auszugehen.
    SystemState state;
    // 1. Error Handling
    if (snapshot.criticalErrorOccured)
    {
        state.systemMode = SystemState::SystemMode::SAFE;
    };

    return state;
}
#endif

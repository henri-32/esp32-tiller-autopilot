#include "diagnostics/diagnostics.h"
#include "diagnostics/diagnostic_types.h"

void Diagnostics::emit(DiagnosticEvent event,
                       FunctionalCapability capability,
                       uint32_t loopTimestamp)
{
    pushEvent(event, capability, loopTimestamp);
    updateCapabilityState(event, capability);
}

void Diagnostics::tick(uint32_t /*loopTimestamp*/)
{
    // bewusst leer für jetzt
    // später: Zeitfenster, Snapshot-Vorbereitung, etc.
}

CapabilityState Diagnostics::capabilityState(FunctionalCapability cap) const
{
    return m_states[static_cast<uint8_t>(cap)];
}

DiagnosticSnapshot Diagnostics::snapshot () {
    DiagnosticSnapshot snapshot; 

    for (uint8_t i = 0; i < static_cast<uint8_t>(FunctionalCapability::COUNT); i++) {
        if (m_states[i]== CapabilityState::CRITICAL_ERROR){
            snapshot.criticalErrorOccured = true; 
            snapshot.criticalCapability = static_cast<FunctionalCapability>(i); 
        };
    };

    snapshot.compassState = m_states[static_cast<uint8_t>(FunctionalCapability::COMPASS)];
    snapshot.gpsState = m_states[static_cast<uint8_t>(FunctionalCapability::GPS)];
    snapshot.windState = m_states[static_cast<uint8_t>(FunctionalCapability::WIND)];
    snapshot.stwState = m_states[static_cast<uint8_t>(FunctionalCapability::STW)];
    snapshot.pwmState = m_states[static_cast<uint8_t>(FunctionalCapability::PWM)]; 
    snapshot.aisState = m_states[static_cast<uint8_t>(FunctionalCapability::AIS)];
    snapshot.countState = m_states[static_cast<uint8_t>(FunctionalCapability::COUNT)];
     
return snapshot;
};

void Diagnostics::pushEvent(DiagnosticEvent event,
                            FunctionalCapability capability,
                            uint32_t ts)
{
    m_events[m_writeIndex] = { event, capability, ts };
    m_writeIndex = (m_writeIndex + 1) % kEventBufferSize;
    if (m_count < kEventBufferSize)
        ++m_count;
}

void Diagnostics::updateCapabilityState(DiagnosticEvent event,
                                        FunctionalCapability capability)
{
    auto& state = m_states[static_cast<uint8_t>(capability)];

    // TODO(Architektur):
    // Mapping bewusst zentral in dieser Funktion pflegen. Wenn neue Events
    // dazukommen, hier sofort in einen CapabilityState uebersetzen.
    switch (event) {
        case DiagnosticEvent::Lost:
            state = CapabilityState::Lost; // latched
            break;

        case DiagnosticEvent::Degraded:
            if (state == CapabilityState::OK)
                state = CapabilityState::Degraded;
            break;

        default:
            // andere Events verändern den Zustand nicht
            break;
    }
}

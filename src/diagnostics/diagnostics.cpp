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

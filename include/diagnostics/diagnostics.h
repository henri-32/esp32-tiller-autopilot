#pragma once
#include <cstdint>
#include "diagnostics/diagnostic_types.h"

struct DiagnosticEntry {
    DiagnosticEvent event;
    FunctionalCapability capability;
    uint32_t timestamp_ms;
};
class Diagnostics {
public:
    static constexpr uint8_t kEventBufferSize = 32;

    // vom Domain-Code aufgerufen
    void emit(DiagnosticEvent event,
              FunctionalCapability capability,
              uint32_t loopTimestamp);

    // vom SystemController im Tick aufgerufen
    void tick(uint32_t loopTimestamp);

    // für spätere Auswertung (noch minimal)
    CapabilityState capabilityState(FunctionalCapability cap) const;

private:
    // --- Event Ringbuffer ---
    DiagnosticEntry m_events[kEventBufferSize];
    uint8_t m_writeIndex = 0;
    uint8_t m_count = 0;

    // --- Capability-Zustände (latched) ---
    CapabilityState m_states[
        static_cast<uint8_t>(FunctionalCapability::COUNT)
    ];

    void pushEvent(DiagnosticEvent event,
                   FunctionalCapability capability,
                   uint32_t ts);

    void updateCapabilityState(DiagnosticEvent event,
                               FunctionalCapability capability);
};


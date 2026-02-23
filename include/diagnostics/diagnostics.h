#pragma once
#include "diagnostics/diagnostic_types.h"
#include <array>
#include <cstdint>

struct DiagnosticEntry {
  DiagnosticEvent event;
  FunctionalCapability capability;
  uint32_t timestamp_ms;
};

struct DiagnosticSnapshot {
  bool criticalErrorOccured;
  FunctionalCapability criticalCapability;
  CapabilityState compassState = CapabilityState::OK;
  CapabilityState gpsState = CapabilityState::OK;
  CapabilityState windState = CapabilityState::OK;
  CapabilityState stwState = CapabilityState::OK;
  CapabilityState pwmState = CapabilityState::OK;
  CapabilityState aisState = CapabilityState::OK;
  CapabilityState countState = CapabilityState::OK;
};

class Diagnostics {
public:
  // Contract:
  // Purpose: Capture and expose diagnostic events and capability states.
  // Inputs: events emitted by domain code + loop timestamps.
  // Outputs/Side-effects: internal event buffer and latched states.
  static constexpr uint8_t kEventBufferSize = 32;

  // vom Domain-Code aufgerufen
  void emit(DiagnosticEvent event, FunctionalCapability capability,
            uint32_t loopTimestamp);

  // vom SystemController im Tick aufgerufen
  void tick(uint32_t loopTimestamp);

  // für spätere gezielte Auswertung (noch minimal)
  CapabilityState capabilityState(FunctionalCapability cap) const;

  DiagnosticSnapshot snapshot();

private:
  // --- Event Ringbuffer ---
  DiagnosticEntry m_events[kEventBufferSize];
  uint8_t m_writeIndex = 0;
  uint8_t m_count = 0;

  // --- Capability-Zustände (latched) ---
  std::array<CapabilityState, static_cast<uint8_t>(FunctionalCapability::COUNT)>
      m_states;

  void pushEvent(DiagnosticEvent event, FunctionalCapability capability,
                 uint32_t ts);

  void updateCapabilityState(DiagnosticEvent event,
                             FunctionalCapability capability);
};

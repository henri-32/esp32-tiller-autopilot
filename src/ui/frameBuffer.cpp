#include "ui/frameBuffer.h"

DisplayContent::DisplayContent(const NavigationSensors &navsens,
                               const Diagnostics &diagnostics,
                               const SystemState &state)
    : m_navigationSensors(navsens), m_diagnostics(diagnostics),
      m_state(state){};

FrameBuffer DisplayContent::tick(const Intent &intent, uint32_t loopTimestamp) {
  createModel(intent);
  FrameBuffer buffer;
  return buffer;
}

void DisplayContent::createModel(const Intent &intent) {
  model.mode = m_state.systemMode;

  model.panelIntent = intent;

  model.ais = m_diagnostics.capabilityState(FunctionalCapability::AIS);
  model.compass = m_diagnostics.capabilityState(FunctionalCapability::COMPASS);
  model.gps = m_diagnostics.capabilityState(FunctionalCapability::GPS);
  model.hullSpeed =
      m_diagnostics.capabilityState(FunctionalCapability::HullSpeed);
  model.wind = m_diagnostics.capabilityState(FunctionalCapability::WIND);
};
#include "ui/frameBuffer.h"

DisplayContent::DisplayContent(const NavigationSensors &navsens,
                               const Diagnostics &diagnostics)
    : m_navigationSensors(navsens), m_diagnostics(diagnostics){};

FrameBuffer DisplayContent::create(const Intent &intent,
                                   const SystemState &state,
                                   uint32_t loopTimestamp) {
  createModel(intent, state);

  FrameBuffer buffer;
  return buffer;
}

DisplayModel DisplayContent::createModel(const Intent &intent,
                                 const SystemState &state) {
  DisplayModel model; 
  model.mode = state.systemMode;

  model.panelIntent = intent;

  model.ais = m_diagnostics.capabilityState(FunctionalCapability::AIS);
  model.compass = m_diagnostics.capabilityState(FunctionalCapability::COMPASS);
  model.gps = m_diagnostics.capabilityState(FunctionalCapability::GPS);
  model.hullSpeed =
      m_diagnostics.capabilityState(FunctionalCapability::STW);
  model.wind = m_diagnostics.capabilityState(FunctionalCapability::WIND);
  return model;
};
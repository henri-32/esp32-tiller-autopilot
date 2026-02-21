#pragma once
#include "diagnostics/diagnostic_types.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

struct DisplayModel {
  SystemState::SystemMode mode = SystemState::SystemMode::OK;
  Intent panelIntent;

  CapabilityState gps = CapabilityState::OK;
  CapabilityState wind = CapabilityState::OK;
  CapabilityState compass = CapabilityState::OK;
  CapabilityState ais = CapabilityState::OK;
  CapabilityState hullSpeed = CapabilityState::OK;
};

struct FrameBuffer {};

class DisplayContent {
public:
  DisplayContent(const NavigationSensors &navsens,
                 const Diagnostics &diagnostics, const SystemState &state);

  FrameBuffer tick(const Intent &intent, uint32_t loopTimestamp);

private:
  DisplayModel model;
  const NavigationSensors &m_navigationSensors;
  const Diagnostics &m_diagnostics;
  const SystemState &m_state;

  void createModel(const Intent &intent);
};
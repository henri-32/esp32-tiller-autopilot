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
  CapabilityState stw = CapabilityState::OK;
};

struct FrameBuffer {};

class UIContent {
public:
  UIContent() = default;

  FrameBuffer create(const Intent &panel,
                     const NavigationSensors::NavigationSnapshot &navigation,
                     const DiagnosticSnapshot &diagnostics,
                     SystemState::SystemMode state, uint32_t loopTimestamp);

private:
  DisplayModel createModel(const Intent &panel,
                           const NavigationSensors::NavigationSnapshot &navigation,
                           const DiagnosticSnapshot &diagnostics,
                           SystemState::SystemMode state);

  FrameBuffer createBuffer(const DisplayModel& model);
};

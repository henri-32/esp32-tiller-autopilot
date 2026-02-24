#include "ui/frameBuffer.h"

FrameBuffer UIContent::create(const Intent &panel,
                              const NavigationSensors::NavigationSnapshot &navigation,
                              const DiagnosticSnapshot &diagnostics,
                              SystemState::SystemMode mode,
                              uint32_t loopTimestamp) {
  const auto model = createModel(panel, navigation, diagnostics, mode);
  const auto buffer = createBuffer(model);

  return buffer;
}

DisplayModel UIContent::createModel(
    const Intent &panel,
    const NavigationSensors::NavigationSnapshot &navigation,
    const DiagnosticSnapshot &diagnostics, SystemState::SystemMode mode) {
  DisplayModel model;

  model.panelIntent = panel;

  model.ais = diagnostics.aisState;
  model.compass = diagnostics.compassState;
  model.gps = diagnostics.gpsState;
  model.stw = diagnostics.stwState;
  model.wind = diagnostics.windState;

  model.mode = mode;

  return model;
};

FrameBuffer UIContent::createBuffer(const DisplayModel &model) {
  FrameBuffer buffer;

  return buffer;
};

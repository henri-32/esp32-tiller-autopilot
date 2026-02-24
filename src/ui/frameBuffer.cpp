#include "ui/frameBuffer.h"



FrameBuffer UIContent::create(Intent panel, NavigationSensors::NavigationSnapshot navigation,
                                   DiagnosticSnapshot diagnostics, SystemState::SystemMode mode,
                                   uint32_t loopTimestamp) {
  createModel(panel, navigation, diagnostics, mode);

  FrameBuffer buffer;
  return buffer;
}

DisplayModel UIContent::createModel(Intent panel, NavigationSensors::NavigationSnapshot navigation,
                                 DiagnosticSnapshot diagnostics, SystemState::SystemMode mode) {
  DisplayModel model; 
  
  model.panelIntent = panel;

  model.ais = diagnostics.aisState; 
  model.compass = diagnostics.compassState;
  model.gps = diagnostics.gpsState;
  model.stw =
      diagnostics.stwState;
  model.wind = diagnostics.windState;

  model.mode = mode; 
  
  return model;
};
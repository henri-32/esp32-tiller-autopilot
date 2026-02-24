#pragma once
#include "core/config.h"

#include "core/steering/sourceHandling.h"
#include "core/steering/steeringOrchestrator.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include "ui/controlPanel.h"
#include "ui/display.h"
#include "ui/frameBuffer.h"

class SystemController {
public:
  // Contract:
  // Purpose: Compose modules and run the top-level control loop.
  // Inputs: loop timestamp; internal sensors and UI state.
  // Outputs/Side-effects: drives actuators, diagnostics, and display updates.
  SystemController();

  void tick(uint32_t loopTimestamp);

private:
  // Top-level composition root. Owns partly hardware, config and
  // orchestration.
  SystemState m_state;
  SteeringControllerConfig m_config;

  // Inputs and UI intent
  ControlPanel m_controlPanel{};
  NavigationSensors m_navigationSensors;

  // Control and supervision pipeline
  Diagnostics m_diagnostics;
  SourceHandler m_sourceHandler;
  SteeringOrchestrator m_steeringOrchestrator;

  // Presentation
  UIContent m_displayContent;
  Display m_display;

  // TODO(Architektur):
  // Als reine Transition-Funktion ausbauen (prevState + Inputs -> nextState).
  // So bleibt die Zustandslogik testbar und vom Tick-Ablauf entkoppelt.
  SystemState stateUpdate(const DiagnosticSnapshot &snapshot);
};

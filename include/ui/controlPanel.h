#pragma once

#include "ui/controlPanelTypes.h"
class ControlPanel {
public:
  // Contract:
  // Purpose: Read user intent from the control interface.
  // Inputs: external UI hardware/state.
  // Outputs/Side-effects: returns PanelIntent; no side-effects.

  PanelIntent readIntent() const;

};

#pragma once
#include "types/controllerTypes.h"

class ControlPanel {
public:
  // Contract:
  // Purpose: Read user intent from the control interface.
  // Inputs: external UI hardware/state.
  // Outputs/Side-effects: returns Intent; no side-effects.

  Intent readIntent() const;
};

#pragma once
#include "types/globalTypes.h"
class ControlPanel {
public:
  // Contract:
  // Purpose: Read user intent from the control interface.
  // Inputs: external UI hardware/state.
  // Outputs/Side-effects: returns Intent; no side-effects.

  Intent readIntent() const;
};

#pragma once
#include "ui/frameBuffer.h"
class Display {
public:
    // Contract:
    // Purpose: Render the latest system state to the display.
    // Inputs: internal system state (not shown here).
    // Outputs/Side-effects: updates display hardware.
    void update(const FrameBuffer &buffer);
};

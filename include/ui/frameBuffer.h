#pragma once
#include "diagnostics/diagnostic_types.h"
#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <vector>

struct DisplayModel
{
    SystemState::SystemMode mode = SystemState::SystemMode::OK;
    Intent panelIntent;

    CapabilityState gps = CapabilityState::OK;
    CapabilityState wind = CapabilityState::OK;
    CapabilityState compass = CapabilityState::OK;
    CapabilityState ais = CapabilityState::OK;
    CapabilityState stw = CapabilityState::OK;
};

struct FrameBuffer
{
};

class UIContent
{
  public:
    UIContent() = default;

    FrameBuffer create(const Intent& panel, const NavigationSensors::NavigationSnapshot& navigation,
                       const DiagnosticSnapshot& diagnostics, SystemState::SystemMode state,
                       uint32_t loopTimestamp);

  private:
    DisplayModel createModel(const Intent& panel,
                             const NavigationSensors::NavigationSnapshot& navigation,
                             const DiagnosticSnapshot& diagnostics, SystemState::SystemMode state);

    FrameBuffer createBuffer(const DisplayModel& model);
};

class BufferBuilderTEST
{
  public:
    BufferBuilderTEST(uint16_t width, uint16_t height);

    void setPixel(int x, int y, bool on);
    void clear();
    void drawChar(int startX, int startY, const uint8_t glyph[8]);
    void drawText(int startX, int startY, char* text);

  private:
    uint16_t m_width;
    uint16_t m_height;
    std::vector<uint8_t> m_buffer;
};

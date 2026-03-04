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

class UIContent
{
  public:
    UIContent(uint16_t width, uint16_t height);

    std::vector<uint8_t> create(const Intent& panel,
                                const NavigationSensors::NavigationSnapshot& navigation,
                                const DiagnosticSnapshot& diagnostics,
                                SystemState::SystemMode state, uint32_t loopTimestamp);

  private:
    DisplayModel createModel(const Intent& panel,
                             const NavigationSensors::NavigationSnapshot& navigation,
                             const DiagnosticSnapshot& diagnostics, SystemState::SystemMode state);

    std::vector<uint8_t> createBuffer(const DisplayModel& Model);

    uint16_t m_width;
    uint16_t m_height;
    uint32_t m_uiCursor;

    // TODO Größe des Pixel Buffers berechnen um heap allocation zu vermeiden
    std::vector<uint8_t> m_buffer;

    void setPixel(int x, int y, bool on);
    void clear();
    void drawChar(int startX, int startY, const uint8_t glyph[8]);
    void drawText(int startX, int startY, const char* const text);
};

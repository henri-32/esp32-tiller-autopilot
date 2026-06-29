#pragma once
#include "config/config.h"
#include "diagnostics/diagnostics.h"
#include "drivers/navigationSensors.h"
#include "types/sensorTypes.h"
#include "types/controllerTypes.h"
#include "types/steeringTypes.h"
#include <cstdint>
#include <vector>

struct DisplayModel
{
    SystemState::SystemMode mode = SystemState::SystemMode::OK;
    Intent panelIntent;
    NavigationSensors::NavigationSnapshot navSnapshot;
    DiagnosticSnapshot diagSnapshot;
};

class UIContent
{
  public:
    static constexpr int kGlyphWidthPx = 16;
    static constexpr int kCharSpacingPx = 3;

    UIContent(const DisplayConfig& config);

    const std::vector<uint8_t>renderBuffer(const Intent& panel,
                                      const NavigationSensors::NavigationSnapshot& navigation,
                                      const DiagnosticSnapshot& diagnostics,
                                      SystemState::SystemMode state, const DisplayConfig& config,
                                      uint32_t loopTimestamp);

  private:
    uint16_t m_height;
    uint16_t m_width;
    uint32_t m_xCursor;
    uint32_t m_yCursor;
    // TODO Größe des Pixel Buffers berechnen um vector zu vermeiden
    std::vector<uint8_t> m_buffer;
    uint32_t m_lastUpdate;

    DisplayModel createModel(const Intent& panel,
                             const NavigationSensors::NavigationSnapshot& navigation,
                             const DiagnosticSnapshot& diagnostics, SystemState::SystemMode state);

    std::vector<uint8_t> createBuffer(const DisplayModel& Model);
    void clear();
    void drawText(int startX, int startY, const char* const text, uint8_t scale = 1, uint8_t spacingScale = 1);
    void drawHorizontalLine(int startX, int startY, int thickness, int length, bool onOff);
    void drawVerticalLine(int startX, int startY, int thickness, int length, bool onOff);
    void renderSourceInformation(const DisplayModel& model);
    void renderCoreInformation(const DisplayModel& model);
    int textWidthPx(const char* text, uint8_t scale) const;
    void drawChar(int startX, int startY, const uint16_t glyph[16], uint8_t scale);
    void setPixel(int x, int y, bool on);
};

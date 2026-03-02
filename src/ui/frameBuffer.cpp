#include "ui/frameBuffer.h"
#include "ui/fonts.h"
#include <cstdint>
#include <optional>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

// ================= UIContent =================

FrameBuffer UIContent::create(const Intent& panel,
                              const NavigationSensors::NavigationSnapshot& navigation,
                              const DiagnosticSnapshot& diagnostics, SystemState::SystemMode mode,
                              uint32_t loopTimestamp)
{
    const auto model = createModel(panel, navigation, diagnostics, mode);
    const auto buffer = createBuffer(model);
    return buffer;
}

DisplayModel UIContent::createModel(const Intent& panel,
                                    const NavigationSensors::NavigationSnapshot& navigation,
                                    const DiagnosticSnapshot& diagnostics,
                                    SystemState::SystemMode mode)
{
    DisplayModel model;

    model.panelIntent = panel;

    model.ais = diagnostics.aisState;
    model.compass = diagnostics.compassState;
    model.gps = diagnostics.gpsState;
    model.stw = diagnostics.stwState;
    model.wind = diagnostics.windState;

    model.mode = mode;

    return model;
}

// ================= FrameBuffer =================
BufferBuilderTEST::BufferBuilderTEST(uint16_t width, uint16_t height)
    : m_width(width), m_height(height),
      m_buffer((static_cast<size_t>(width) * static_cast<size_t>(height) + 7) / 8){};

void BufferBuilderTEST::setPixel(int x, int y, bool on)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;

    size_t pixelIndex = static_cast<size_t>(y) * m_width + x;
    size_t byteIndex = pixelIndex / 8;
    uint8_t bitOffset = pixelIndex % 8;

    if (byteIndex >= m_buffer.size())
        return;

    uint8_t& byte = m_buffer[byteIndex];

    if (on)
        byte |= 1u << (7 - bitOffset);
    else
        byte &= ~(1u << (7 - bitOffset));
}

void BufferBuilderTEST::clear()
{
    for (auto& b : m_buffer)
        b = 0;
}

void BufferBuilderTEST::drawChar(int startX, int startY, const uint8_t glyph[8])
{
    for (int row = 0; row < 8; ++row)
    {
        uint8_t binary = glyph[row];

        for (int col = 0; col < 8; ++col)
        {
            uint8_t mask = 1u << (7 - col);

            if (binary & mask)
                setPixel(startX + col, startY + row, true);
            else
                setPixel(startX + col, startY + row, false);
        }
    }
};

void BufferBuilderTEST::drawText(int startX, int startY, char* text)
{
    const uint8_t pixelsBtwChars = 3;
    uint8_t cursorX = startX;

    for (int i = 0; text[i] != '\0'; ++i)
    {
        char c = text[i];
        drawChar(cursorX, startY, fontDigits[c - '0']);
        cursorX += 8 + pixelsBtwChars;
    };
};

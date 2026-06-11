#include "ui/uiContent.h"
#include "ui/displayModelExpressions.h"
#include "ui/fonts.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <vector>

// ================= UIContent =================

UIContent::UIContent(const DisplayConfig& config)

    : m_width(config.displayWidth), m_height(config.displayHeight),
      m_buffer(
          (static_cast<size_t>(config.displayWidth) * static_cast<size_t>(config.displayHeight) +
           7) /
          8){};

const std::vector<uint8_t>
UIContent::renderBuffer(const Intent& panel,
                        const NavigationSensors::NavigationSnapshot& navigation,
                        const DiagnosticSnapshot& diagnostics, SystemState::SystemMode mode,
                        const DisplayConfig& config, uint32_t loopTimestamp)
{
    if (loopTimestamp - m_lastUpdate >= config.refreshRate_ms)
    {
        auto model = createModel(panel, navigation, diagnostics, mode);
        const auto buffer = createBuffer(model);
    };

    return m_buffer;
};

DisplayModel UIContent::createModel(const Intent& panel,
                                    const NavigationSensors::NavigationSnapshot& navigation,
                                    const DiagnosticSnapshot& diagnostics,
                                    SystemState::SystemMode mode)
{
    DisplayModel model;

    model.panelIntent = panel;
    model.navSnapshot = navigation;
    model.diagSnapshot = diagnostics;
    model.mode = mode;

    // ================================================================
    // TODO Für Testzwecke kann hier das Ausgansmodell verändert werden.
    model.diagSnapshot.aisState = CapabilityState::Lost;
    model.navSnapshot.LeadSource = NavigationSource::Gps;
    model.navSnapshot.compass_hdg_dg.value = 359;
    //=================================================================
    return model;
};

std::vector<uint8_t> UIContent::createBuffer(const DisplayModel& model)
{
    clear();
    m_xCursor = 0;
    m_yCursor = 0;

    renderSourceInformation(model);
    renderCoreInformation(model);

    return m_buffer;
};

void UIContent::clear()
{
    for (auto& b : m_buffer)
        b = 0;
}

void UIContent::renderSourceInformation(const DisplayModel& model)
{
    constexpr int pixelBtwWords = 96;
    constexpr int pixelBtwRows = 4;
    uint8_t scale = 2;
    // Stabile DarstellungSources in Zeile 1
    int begin_compass_column = 0;

    drawText(0, 0, expression::Compass, scale);
    int begin_gps_column = m_xCursor += textWidthPx(expression::Compass, scale) + pixelBtwWords;

    drawText(m_xCursor, 0, expression::GPS, scale);
    int begin_wind_column = m_xCursor += textWidthPx(expression::GPS, scale) + pixelBtwWords;

    drawText(m_xCursor, 0, expression::Wind, scale);
    int begin_ais_column = m_xCursor += textWidthPx(expression::Wind, scale) + pixelBtwWords;

    drawText(m_xCursor, 0, expression::AIS, scale);
    int begin_stw_column = m_xCursor += textWidthPx(expression::AIS, scale) + pixelBtwWords;

    drawText(m_xCursor, 0, expression::STW, scale);
    m_xCursor += textWidthPx(expression::STW, scale) + pixelBtwWords;

    // Zeilenende
    m_xCursor = 0;
    m_yCursor += kGlyphWidthPx * scale + pixelBtwRows; // Höhe der Zeichen* last scale + pbr

    drawHorizontalLine(0, m_yCursor, 3, 1080, true);
    m_yCursor += 3; // Höhe der Linie(thickness)
    scale = 1;
    // SourceStates
    switch (model.diagSnapshot.compassState)
    {
    case CapabilityState::OK:
        drawText(0, m_yCursor, expression::OK, scale);
        break;

    case CapabilityState::Degraded:
        drawText(0, m_yCursor, expression::Degraded, scale);
        break;

    case CapabilityState::Lost:
        drawText(0, m_yCursor, expression::Lost, scale);
        break;

    case CapabilityState::LostWithFallback:
        drawText(0, m_yCursor, expression::LostWithFallback, scale);
        break;

    default:
        break;
    };

    switch (model.diagSnapshot.aisState)
    {
    case CapabilityState::OK:
        drawText(begin_gps_column, m_yCursor, expression::OK, scale);
        break;

    case CapabilityState::Degraded:
        drawText(begin_gps_column, m_yCursor, expression::Degraded, scale);
        break;

    case CapabilityState::Lost:
        drawText(begin_gps_column, m_yCursor, expression::Lost, scale);
        break;

    case CapabilityState::LostWithFallback:
        drawText(begin_gps_column, m_yCursor, expression::LostWithFallback, scale);
        break;

    default:
        break;
    };

    switch (model.diagSnapshot.windState)
    {
    case CapabilityState::OK:
        drawText(begin_wind_column, m_yCursor, expression::OK, scale);
        break;

    case CapabilityState::Degraded:
        drawText(begin_wind_column, m_yCursor, expression::Degraded, scale);
        break;

    case CapabilityState::Lost:
        drawText(begin_wind_column, m_yCursor, expression::Lost, scale);
        break;

    case CapabilityState::LostWithFallback:
        drawText(begin_wind_column, m_yCursor, expression::LostWithFallback, scale);
        break;

    default:
        break;
    };

    switch (model.diagSnapshot.aisState)
    {
    case CapabilityState::OK:
        drawText(begin_ais_column, m_yCursor, expression::OK, scale);
        break;

    case CapabilityState::Degraded:
        drawText(begin_ais_column, m_yCursor, expression::Degraded, scale);
        break;

    case CapabilityState::Lost:
        drawText(begin_ais_column, m_yCursor, expression::Lost, scale);
        break;

    case CapabilityState::LostWithFallback:
        drawText(begin_ais_column, m_yCursor, expression::LostWithFallback, scale);
        break;

    default:
        break;
    };

    switch (model.diagSnapshot.stwState)
    {
    case CapabilityState::OK:
        drawText(begin_stw_column, m_yCursor, expression::OK, scale);
        break;

    case CapabilityState::Degraded:
        drawText(begin_stw_column, m_yCursor, expression::Degraded, scale);
        break;

    case CapabilityState::Lost:
        drawText(begin_stw_column, m_yCursor, expression::Lost, scale);
        break;

    case CapabilityState::LostWithFallback:
        drawText(begin_stw_column, m_yCursor, expression::LostWithFallback, scale);
        break;
    };

    m_yCursor += kGlyphWidthPx * scale + pixelBtwRows;
    drawHorizontalLine(0, m_yCursor, 3, 1080, true);
    m_yCursor += 3;

    switch (model.navSnapshot.LeadSource)
    {
    case NavigationSource::Compass:
        drawText(begin_compass_column, m_yCursor, expression::LeadSource, scale);
        break;

    case NavigationSource::Gps:
        drawText(begin_gps_column, m_yCursor, expression::LeadSource, scale);
        break;

    case NavigationSource::Wind:
        drawText(begin_wind_column, m_yCursor, expression::LeadSource, scale);
        break;

    default:
        break;
    }
	m_yCursor += kGlyphWidthPx * scale; 
	m_xCursor = 0; 

	drawHorizontalLine(m_xCursor, m_yCursor, 5, 1080, true); 
};

void UIContent::renderCoreInformation(const DisplayModel& model)
{
    m_yCursor = 350;
    m_xCursor = 0;
    uint8_t scale = 2;
	uint8_t spacingScale = 2; 
    drawText(m_xCursor, m_yCursor, expression::Target, scale);
    m_yCursor += kGlyphWidthPx * scale;

    scale = 4;
    uint16_t current = model.navSnapshot.compass_hdg_dg.value;
    // Der Buffer müsste 4 chars beinhalten können. 3 für die HDG Zahlen, 1 für Nullterminator
    // Damit gibt es aber eine compilerwarnung, weil uint16_t größer sein kann und bis zu 6 Bytes im
    // Buffer brauchen könnte. Deswegen auf 6 erhöht.
    char Buffer[6];
    snprintf(Buffer, sizeof(Buffer), "%u", current);
    drawHorizontalLine(m_xCursor, m_yCursor, 5, textWidthPx("359", scale) + 30, true);
    m_yCursor += 5;
    drawText(m_xCursor, m_yCursor, Buffer, scale);
    m_xCursor += textWidthPx("359", scale) + 25;
    drawVerticalLine(m_xCursor, m_yCursor, 5, kGlyphWidthPx * scale, true);
    m_yCursor += kGlyphWidthPx * scale;
    drawHorizontalLine(0, m_yCursor, 5, m_xCursor + 5, true);
    m_xCursor = 0;
};

void UIContent::setPixel(int x, int y, bool on)
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

void UIContent::drawChar(int startX, int startY, const uint16_t glyph[16], uint8_t scale)
{
    for (int row = 0; row < 16; ++row)
    {
        uint16_t binary = glyph[row];

        for (int col = 0; col < 16; ++col)
        {
            uint16_t mask = static_cast<uint16_t>(1u << col);

            const bool on = (binary & mask) != 0;
            for (int dy = 0; dy < scale; ++dy)
            {
                for (int dx = 0; dx < scale; ++dx)
                {
                    setPixel(startX + col * scale + dx, startY + row * scale + dy, on);
                }
            }
        }
    }
};

void UIContent::drawText(int startX, int startY, const char* const text, uint8_t scale, uint8_t spacingScale)
{
	//Default ist, dass das spacing mitskaliert
	spacingScale = scale; 

    if (scale <= 0 || scale >= 10)
    {
        scale = 1;
    };

    int cursorX = startX;

    for (int i = 0; text[i] != '\0'; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        drawChar(cursorX, startY, asciiFont16x16[c].data(), scale);
        cursorX += kGlyphWidthPx * scale + kCharSpacingPx * spacingScale;
    };
};

void UIContent::drawHorizontalLine(int startX, int startY, int thickness, int length, bool onOff)
{
    int y = startY;

    for (int i = 0; i < thickness; i++)
    {
        int x = startX;
        for (int ii = 0; ii < length; ii++)
        {
            setPixel(x, y, onOff);
            x++;
        };
        y++;
    };
};

void UIContent::drawVerticalLine(int startX, int startY, int thickness, int length, bool onOff)
{
    int x = startX;
    for (int i = 0; i < thickness; i++)
    {
        int y = startY;
        for (int ii = 0; ii < length; ii++)
        {
            setPixel(x, y, onOff);
            y++;
        };
        x++;
    };
};

int UIContent::textWidthPx(const char* text, uint8_t scale) const
{
    const int len = static_cast<int>(std::strlen(text));
    if (len <= 0)
        return 0;
    return len * ((kGlyphWidthPx + kCharSpacingPx) - kCharSpacingPx) * scale;
};

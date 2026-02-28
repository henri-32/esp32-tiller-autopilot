#include "ui/frameBuffer.h"
#include <vector>

FrameBuffer
UIContent::create(const Intent &panel,
                  const NavigationSensors::NavigationSnapshot &navigation,
                  const DiagnosticSnapshot &diagnostics,
                  SystemState::SystemMode mode, uint32_t loopTimestamp) {
  const auto model = createModel(panel, navigation, diagnostics, mode);
  const auto buffer = createBuffer(model);

  return buffer;
}

DisplayModel
UIContent::createModel(const Intent &panel,
                       const NavigationSensors::NavigationSnapshot &navigation,
                       const DiagnosticSnapshot &diagnostics,
                       SystemState::SystemMode mode) {
  DisplayModel model;

  model.panelIntent = panel;

  model.ais = diagnostics.aisState;
  model.compass = diagnostics.compassState;
  model.gps = diagnostics.gpsState;
  model.stw = diagnostics.stwState;
  model.wind = diagnostics.windState;

  model.mode = mode;

  return model;
};

FrameBuffer UIContent::createBuffer(const DisplayModel &model) {
  FrameBuffer buffer;

  return buffer;
};

class FRAMEBUFFER {
public:
  FRAMEBUFFER(uint16_t width, uint16_t height)
      : m_width(width), m_height(height),
        m_buffer(
            (static_cast<size_t>(width) * static_cast<size_t>(height) + 7) /
            8){};

  void setPixel(int x, int y, bool on) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
      return;
    }

    size_t pixelIndex = static_cast<size_t>(y) * m_width + x;
    uint16_t byteIndex = pixelIndex / 8;
    uint8_t bitOffset = pixelIndex % 8;

    if (byteIndex >= m_buffer.size()) {
      return;
    }

    uint8_t &byte = m_buffer[byteIndex];

    if (on) {
      byte |= 1u << (7 - bitOffset);
    }

    else if (!on) {
      byte &= ~(1u << (7 - bitOffset));
    }
   
  };

  void clear() {
    uint32_t bufferSize = m_buffer.size();

    for (int i = 0; i < bufferSize; i++) {
      uint8_t byte = m_buffer[i];
      uint8_t bitmask = ~byte;
      byte &= bitmask;
      m_buffer[i] = byte;
    }
  };

private:
  uint16_t m_width;
  uint16_t m_height;

  std::vector<uint8_t> m_buffer;
};

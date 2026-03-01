#include "ui/frameBuffer.h"
#include <vector>
#include <optional>

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

constexpr uint8_t fontDigits[10][8] = {

    // '0'
    {
        0b00111100,
        0b01000010,
        0b01000110,
        0b01001010,
        0b01010010,
        0b01100010,
        0b00111100,
        0b00000000
    },

    // '1'
    {
        0b00011000,
        0b00101000,
        0b01001000,
        0b00001000,
        0b00001000,
        0b00001000,
        0b01111110,
        0b00000000
    },

    // '2'
    {
        0b00111100,
        0b01000010,
        0b00000010,
        0b00001100,
        0b00110000,
        0b01000000,
        0b01111110,
        0b00000000
    },

    // '3'
    {
        0b00111100,
        0b01000010,
        0b00000010,
        0b00011100,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000
    },

    // '4'
    {
        0b00000100,
        0b00001100,
        0b00010100,
        0b00100100,
        0b01000100,
        0b01111110,
        0b00000100,
        0b00000000
    },

    // '5'
    {
        0b01111110,
        0b01000000,
        0b01111100,
        0b00000010,
        0b00000010,
        0b01000010,
        0b00111100,
        0b00000000
    },

    // '6'
    {
        0b00111100,
        0b01000000,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000
    },

    // '7'
    {
        0b01111110,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00000000
    },

    // '8'
    {
        0b00111100,
        0b01000010,
        0b01000010,
        0b00111100,
        0b01000010,
        0b01000010,
        0b00111100,
        0b00000000
    },

    // '9'
    {
        0b00111100,
        0b01000010,
        0b01000010,
        0b00111110,
        0b00000010,
        0b00000100,
        0b00111000,
        0b00000000
    }
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


      std::optional <const uint8_t*> getGlyph(char c) {
        char allowed[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; 
        bool isAllowed = false; 

        for (int i = 0; i < 10; i++) {
            if (c == allowed[i]) {
                uint8_t font = fontDigits[i]; 
                isAllowed = true;}

        if (!isAllowed) {return std::nullopt;}
        else {return font};  
         
  };
  void drawChar(int startX, int startY, const uint8_t  glyph[8]){
      for (int i = 0; i < 8 ; i++) {
        uint8_t binary = glyph[i];
        uint8_t row = i; 

        for (int ii = 0; ii < 8; ii++) {
           uint8_t mask = 1 << (7 - ii);  

            if(binary & mask) {
            setPixel(startX + ii, startY + row, true);
            } else if (!(binary & mask)) {
                setPixel(startX + ii, startY + row, false);
            }; 

        };

      };        
    }; 

private:
  uint16_t m_width;
  uint16_t m_height;

  std::vector<uint8_t> m_buffer;
};

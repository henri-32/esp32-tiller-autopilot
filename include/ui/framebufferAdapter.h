#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class Framebuffer1BitToARGBAdapter
{
  public:
    Framebuffer1BitToARGBAdapter(int width, int height)
        : m_width(width), m_height(height),
          m_argb(static_cast<size_t>(width) * static_cast<size_t>(height), 0xFF000000u)
    {
    }

    const uint32_t* convert(const std::vector<uint8_t>& packed1Bit)
    {
        const size_t pixelCount = static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
        const size_t requiredPackedBytes = (pixelCount + 7u) / 8u;

        if (packed1Bit.size() < requiredPackedBytes)
            return nullptr;

        for (size_t i = 0; i < pixelCount; ++i)
        {
            const uint8_t byte = packed1Bit[i / 8u];
            const uint8_t bitIndex = static_cast<uint8_t>(7u - (i % 8u));
            const bool on = ((byte >> bitIndex) & 0x1u) != 0u;
            m_argb[i] = on ? 0xFFFFFFFFu : 0xFF000000u;
        }

        return m_argb.data();
    }

    int pitchBytes() const
    {
        return m_width * static_cast<int>(sizeof(uint32_t));
    }

  private:
    int m_width;
    int m_height;
    std::vector<uint32_t> m_argb;
};

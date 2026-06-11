#pragma once
#include <SDL2/SDL.h>
#include <ui/framebufferAdapter.h>
#include <vector>
#include <optional>

class nativeGUIRendering
{
  public:
    nativeGUIRendering()
        : m_window(nullptr), m_renderer(nullptr), m_texture(nullptr),
          m_fbAdapter(WIDTH, HEIGHT)
    {
    }

    ~nativeGUIRendering()
    {
        if (m_texture)
            SDL_DestroyTexture(m_texture);
        if (m_renderer)
            SDL_DestroyRenderer(m_renderer);
        if (m_window)
            SDL_DestroyWindow(m_window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    void init()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            return;

        m_window = SDL_CreateWindow("Autopilot GUI", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
        if (!m_window)
            return;

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        if (!m_renderer)
            m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_renderer)
            return;

        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    }

    int tick(const std::vector<uint8_t>& framebuffer1Bit)
    {
        if (!m_window || !m_renderer || !m_texture)
            return 0;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;
        }

        const uint32_t* pixels = m_fbAdapter.convert(framebuffer1Bit);
        if (!pixels)
            return 0;

        if (SDL_UpdateTexture(m_texture, nullptr, pixels, m_fbAdapter.pitchBytes()) != 0)
            return 0;

        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(16);
        return 1;
    }

  private:
    static constexpr int WIDTH = 1080;
    static constexpr int HEIGHT = 1440;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;
    Framebuffer1BitToARGBAdapter m_fbAdapter;
};

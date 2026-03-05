#pragma once
#include <SDL2/SDL.h>


class nativeGUIRendering
{
  public:
    nativeGUIRendering();

    void tick()
    {
        SDL_Window* window = SDL_CreateWindow("Autopilot GUI", SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                                 SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

        bool running = true;
        while (running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }
            }
        };
    };

  private:
    static constexpr int WIDTH = 600;
    static constexpr int HEIGHT = 600;
};

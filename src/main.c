#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"

bool is_running = false;

void setup()
{
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
}

void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

void update()
{
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // clear_color_buffer(0xFFFFFF00);
    clear_color_buffer(0xFF111111);
    // draw_grid(0xFF333333, 100);
    draw_grid_dots(0xFF888888, 10);
    // draw_rect_xy(100, 300, 800, 500, 0xFF0000FF);
    draw_rect(100, 300, 800, 500, 0xFF0000FF);
    render_color_buffer();

    SDL_RenderPresent(renderer);
}

int main(void)
{
    is_running = initialize_window(); 
    setup();
    while (is_running)
    {
        process_input();
        update();
        render();
    }
    destroy_window();
    return 0;
}


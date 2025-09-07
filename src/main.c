#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool is_running = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

bool initialize_window()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;
    window = SDL_CreateWindow(
        NULL, 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window)
    {
        fprintf(stderr, "Error creating SDL window\n");
        return false;
    }
    renderer = SDL_CreateRenderer(
        window,
        -1,
        0
    );
    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL renderer\n");
        return false;
    }
    // real fullscreen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    return true;
} 

void draw_pixel(int x, int y, uint32_t color)
{
    color_buffer[(window_width * y) + x] = color;
}

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

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            draw_pixel(x, y, color);
        }
    }
}

void draw_grid_dots(uint32_t color, int grid_size)
{
    for (int y = 0; y < window_height; y+=grid_size)
    {
        for (int x = 0; x < window_width; x+=grid_size)
        {
            draw_pixel(x, y, color);
        }
    }
}

void draw_grid(uint32_t color, int grid_size)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            if (x % grid_size == 0 || y % grid_size == 0)
            {
                draw_pixel(x, y, color);
            }
        }
    }
}

void draw_rect_xy(int x1, int y1, int x2, int y2, uint32_t color)
{
    for (int y = y1; y < y2; y++)
    {
        for (int x = x1; x < x2; x++)
        {
            draw_pixel(x, y, color);
        }
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int k = y; k < y+h; k++)
    {
        for (int i = x; i < x+w; i++)
        {
            draw_pixel(i, k, color);
        }
    }
}

void render_color_buffer() {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)(window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
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

void destroy_window()
{
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
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


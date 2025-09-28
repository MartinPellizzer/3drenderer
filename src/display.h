#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern uint32_t *color_buffer;
extern float *z_buffer;
extern SDL_Texture *color_buffer_texture;
extern int window_width;
extern int window_height;

bool initialize_window();
void draw_pixel(int x, int y, uint32_t color);
void clear_color_buffer(uint32_t color);
void clear_z_buffer();
void draw_grid_dots(uint32_t color, int grid_size);
void draw_grid(uint32_t color, int grid_size);
void draw_rect_xy(int x1, int y1, int x2, int y2, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void render_color_buffer();
void destroy_window();

#endif

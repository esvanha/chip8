#ifndef IO_H_
#define IO_H_

#include "error.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

#define DISPLAY_WIDTH  64
#define DISPLAY_HEIGHT 32
#define PIXEL_SIZE     16
#define WINDOW_WIDTH   (DISPLAY_WIDTH * PIXEL_SIZE)
#define WINDOW_HEIGHT  (DISPLAY_HEIGHT * PIXEL_SIZE)
#define FPS            60

struct IO {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool pixel_map[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    unsigned int ticks_at_last_draw;
};

enum Error io_init(struct IO*);
enum Error io_update_display(struct IO*);
enum Error io_clear_display(struct IO*);
bool       io_is_key_pressed(int8_t);
int8_t     io_pressed_key();
void       io_beep();
void       io_quit(struct IO*);

#endif

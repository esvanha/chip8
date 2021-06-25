#include "io.h"

enum Error
io_init(struct IO* io)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return E_SDL_ERROR;

    SDL_Window* window = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    io->window = window;
    if (window == NULL)
        goto error;

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );
    io->renderer = renderer;
    if (renderer == NULL)
        goto error;

    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) != 0)
        goto error;
    if (SDL_RenderClear(renderer) != 0)
        goto error;
    SDL_RenderPresent(renderer);

    memset(io->pixel_map, false, sizeof(io->pixel_map));
    io->ticks_at_last_draw = 0;
    
    return E_OK;

    error:
        io_quit(io);
        return E_SDL_ERROR;
}

enum Error
io_clear_display(struct IO* io)
{
    memset(io->pixel_map, false, sizeof(io->pixel_map));
    
    if (SDL_SetRenderDrawColor(io->renderer, 0, 0, 0, 0) != 0)
        return E_SDL_ERROR;
    if (SDL_RenderClear(io->renderer) != 0)
        return E_SDL_ERROR;

    return E_OK;
}

enum Error
io_update_display(struct IO* io)
{
    //.. Maintain a stable FPS
    const unsigned int ticks_since_last_draw = (
        SDL_GetTicks() - io->ticks_at_last_draw
    );
    if (ticks_since_last_draw < (1000/FPS)) {
        SDL_Delay((1000/FPS) - ticks_since_last_draw);
    }

    if (SDL_SetRenderDrawColor(io->renderer, 0, 0, 0, 0) != 0)
        return E_SDL_ERROR;
    if (SDL_RenderClear(io->renderer) != 0)
        return E_SDL_ERROR;

    if (SDL_SetRenderDrawColor(io->renderer, 255, 255, 255, 255) != 0)
        return E_SDL_ERROR;

    SDL_Rect rect = { .w = PIXEL_SIZE, .h = PIXEL_SIZE };
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            if (io->pixel_map[i][j]) {
                rect.x = j * PIXEL_SIZE;
                rect.y = i * PIXEL_SIZE;
                if (SDL_RenderFillRect(io->renderer, &rect) != 0)
                    return E_SDL_ERROR;
            }
        }
    }

    SDL_RenderPresent(io->renderer);
    io->ticks_at_last_draw = SDL_GetTicks();

    return E_OK;
}

void
io_beep()
{
    putc('\a', stdout);
}

//.. Since CHIP-8 uses a hexademical input keyboard, we need to translate keys
//   on a modern keyboard to a hexademical value. The mapping used assumes
//   a QWERTY layout.
const SDL_Scancode VALUE_TO_KEYBOARD_MAP[] = {
    //.. First row
    [0x1] = SDL_SCANCODE_1, [0x2] = SDL_SCANCODE_2,
    [0x3] = SDL_SCANCODE_3, [0xC] = SDL_SCANCODE_4,
    //.. Second row
    [0x4] = SDL_SCANCODE_Q, [0x5] = SDL_SCANCODE_W,
    [0x6] = SDL_SCANCODE_E, [0xD] = SDL_SCANCODE_R,
    //.. Third row
    [0x7] = SDL_SCANCODE_A, [0x8] = SDL_SCANCODE_S,
    [0x9] = SDL_SCANCODE_D, [0xE] = SDL_SCANCODE_F,
    //.. Fourth row
    [0xA] = SDL_SCANCODE_Z, [0x0] = SDL_SCANCODE_X,
    [0xB] = SDL_SCANCODE_C, [0xF] = SDL_SCANCODE_V,
};

//.. Check if key is pressed whose value corresponds with the given `value`.
//   E.g. when a value of 0xC is passed, key 4 has to be pressed on the
//   keyboard.
bool
io_is_key_pressed(int8_t value)
{
    assert(value >= 0 && value <= 0xF);
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);
    return keyboard_state[VALUE_TO_KEYBOARD_MAP[value]];
}

//.. When multiple keys are pressed, the value of the first one in order from
//   top left to bottom right is returned.
int8_t
io_pressed_key()
{
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);
    //.. First row
    if (keyboard_state[SDL_SCANCODE_1])      return 1;
    else if (keyboard_state[SDL_SCANCODE_2]) return 2;
    else if (keyboard_state[SDL_SCANCODE_3]) return 3;
    else if (keyboard_state[SDL_SCANCODE_4]) return 0xC;
    //.. Second row
    else if (keyboard_state[SDL_SCANCODE_Q]) return 4;
    else if (keyboard_state[SDL_SCANCODE_W]) return 5;
    else if (keyboard_state[SDL_SCANCODE_E]) return 6;
    else if (keyboard_state[SDL_SCANCODE_R]) return 0xD;
    //.. Third row
    else if (keyboard_state[SDL_SCANCODE_A]) return 7;
    else if (keyboard_state[SDL_SCANCODE_S]) return 8;
    else if (keyboard_state[SDL_SCANCODE_D]) return 9;
    else if (keyboard_state[SDL_SCANCODE_F]) return 0xE;
    //.. Fourth row
    else if (keyboard_state[SDL_SCANCODE_Z]) return 0xA;
    else if (keyboard_state[SDL_SCANCODE_X]) return 0;
    else if (keyboard_state[SDL_SCANCODE_C]) return 0xB;
    else if (keyboard_state[SDL_SCANCODE_V]) return 0xF;
    //..
    else                                     return -1;
}

void
io_quit(struct IO* io)
{
    if (io->renderer != NULL)
        SDL_DestroyRenderer(io->renderer);

    if (io->window != NULL)
        SDL_DestroyWindow(io->window);

    SDL_Quit();
}

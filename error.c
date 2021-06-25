#include "error.h"

#include <SDL2/SDL.h>

const char*
error_to_str(enum Error err)
{
    switch (err) {
    case E_VM_STACK_OVERFLOW:
        return "stack overflow in virtual machine";
    case E_VM_STACK_UNDERFLOW:
        return "stack underflow in virtual machine";
    case E_VM_OUT_OF_MEMORY:
        return "virtual machine is out of memory";
    case E_VM_UNKNOWN_UPCODE:
        return "unknown upcode encountered in virtual machine";
    case E_COULDNT_OPEN_FILE:
        return "couldn't open file";
    case E_COULDNT_READ_FILE:
        return "couldn't read file";
    case E_SDL_ERROR:
        return SDL_GetError();
    case E_OK:
        return "OK is not an error.";
    }

    assert(0); //.. This should be unreachable as all errors should be handled.
}

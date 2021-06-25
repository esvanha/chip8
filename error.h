#ifndef ERROR_H_
#define ERROR_H_

#include <assert.h>

enum Error {
    E_OK = 1,
    E_VM_STACK_OVERFLOW,
    E_VM_STACK_UNDERFLOW,
    E_VM_OUT_OF_MEMORY,
    E_VM_UNKNOWN_UPCODE,
    E_COULDNT_OPEN_FILE,
    E_COULDNT_READ_FILE,
    E_SDL_ERROR,
};

const char* error_to_str(enum Error err);

#endif

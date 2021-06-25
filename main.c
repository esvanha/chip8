#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include "vm.h"
#include "error.h"

#define PRINT_ERROR(err) fprintf(stderr, "Error: %s\n", error_to_str(err));

int
main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("CHIP-8 Emulator\nUsage: %s <path to ROM>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct VM vm;
    enum Error err = vm_new(&vm);
    if (err != E_OK) {
        PRINT_ERROR(err);
        return EXIT_FAILURE;
    }

    err = vm_insert_rom(&vm, argv[1]);
    if (err != E_OK) {
        PRINT_ERROR(err);
        vm_quit(&vm);
        return EXIT_FAILURE;
    }

    bool quit_flag = false;
    while ((err = vm_next(&vm, &quit_flag)) == E_OK && !quit_flag) {
        vm_print_debug(vm);
    }
    if (err != E_OK)
        PRINT_ERROR(err);

    vm_quit(&vm);

    return EXIT_SUCCESS;
}

#include "vm.h"
#include "instructions.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

enum Error
chip8stack_push(struct Chip8Stack* cs, uint16_t value)
{
    if (cs->length >= 16)
        return E_VM_STACK_OVERFLOW;

    cs->contents[cs->length++] = value;

    return E_OK;
}

enum Error
chip8stack_pop(struct Chip8Stack* cs, uint16_t* result)
{
    if (cs->length == 0)
        return E_VM_STACK_UNDERFLOW;

    *result = cs->contents[--cs->length];

    return E_OK;
}

enum Error
vm_new(struct VM* vm)
{
    //.. For the RND instruction
    srand(time(NULL));

    *vm = (struct VM) {
        .data_registers = {0},
        .address_register = 0,
        .delay_timer = 0,
        .sound_timer = 0,
        .program_counter = PROGRAM_START,
        .stack = (struct Chip8Stack) {
            .contents = {0},
            .length = 0,
        },
        .memory = {
            /* Load first regions of memory with fonts */
            /* 0 */ 0xF0, 0x90, 0x90, 0x90, 0xF0,
            /* 1 */ 0x20, 0x60, 0x20, 0x20, 0x70,
            /* 2 */ 0xF0, 0x10, 0xF0, 0x80, 0xF0,
            /* 3 */ 0xF0, 0x10, 0xF0, 0x10, 0xF0,
            /* 4 */ 0x90, 0x90, 0xF0, 0x10, 0x10,
            /* 5 */ 0xF0, 0x80, 0xF0, 0x10, 0xF0,
            /* 6 */ 0xF0, 0x80, 0xF0, 0x90, 0xF0,
            /* 7 */ 0xF0, 0x10, 0x20, 0x40, 0x40,
            /* 8 */ 0xF0, 0x90, 0xF0, 0x90, 0xF0,
            /* 9 */ 0xF0, 0x90, 0xF0, 0x10, 0xF0,
            /* A */ 0xF0, 0x90, 0xF0, 0x90, 0x90,
            /* B */ 0xE0, 0x90, 0xE0, 0x90, 0xE0,
            /* C */ 0xF0, 0x80, 0x80, 0x80, 0xF0,
            /* D */ 0xE0, 0x90, 0x90, 0x90, 0xE0,
            /* E */ 0xF0, 0x80, 0xF0, 0x80, 0xF0,
            /* F */ 0xF0, 0x80, 0xF0, 0x80, 0x80,
        },
    };

    return io_init(&vm->io);
}

void
vm_quit(struct VM* vm)
{
    io_quit(&vm->io);
}

enum Error
vm_insert_instruction(struct VM* vm, int16_t instruction)
{
    if (vm->program_counter + 2 > MEMORY_SIZE)
        return E_VM_OUT_OF_MEMORY;

    //.. Insert 16-bit instruction as two 8-bit values
    vm->memory[vm->program_counter++] = instruction >> 8; /* Higher byte */
    vm->memory[vm->program_counter++] = instruction & 0xFF; /* Lower byte */

    return E_OK;
}

void
vm_print_debug(struct VM vm)
{
    puts("-------");
    printf("Program Counter (PC): %d (0x%X)\n", vm.program_counter, vm.program_counter);

    for (uint8_t i = 0; i < REGISTERS_SIZE; i++)
        printf("Register V%d = %d\n", i, vm.data_registers[i]);

    printf("Address Register (I): %d (0x%X)\n", vm.address_register, vm.address_register);
    puts("--------");
}

#define NIBBLES_TO_BYTE(higher_nibble, lower_nibble)\
    ((higher_nibble << 4) + lower_nibble)
#define THREE_NIBBLES_TO_12_BIT(higher_nibble, mid_nibble, lower_nibble)\
    ((higher_nibble << 8) + (mid_nibble << 4) + lower_nibble)

static enum Error
execute_opcode(struct VM* vm, uint16_t opcode)
{
    //.. Nibbles where the highest nibble is the first etc.
    const uint4_t nibble_4 = opcode & 0xF;
    const uint4_t nibble_3 = (opcode >> 4) & 0xF;
    const uint4_t nibble_2 = (opcode >> 8) & 0xF;
    const uint4_t nibble_1 = (opcode >> 12) & 0xF;

    /* Chip-8 instructions taken from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM .
     *
     * All opcodes can be split into six groups, depending on which nibble is variable:
     * - A_NNN;
     * - A_B_C_D;
     * - A_X_KK;
     * - A_X_Y_B;
     * - A_X_Y_N;
     * - A_X_B_C
     * Where:
     * - A, B, C, D are constants;
     * - NNN is a 12-bit value;
     * - N is a nibble;
     * - X and Y are 4-bit values;
     * - KK is a 8-bit value.
     *
     * For every opcode group a macro is defined..
     */

    #define CHECK_OPCODE_A_NNN(a, fn) \
        do {\
            if (nibble_1 == a) {\
                const uint12_t nnn = THREE_NIBBLES_TO_12_BIT(nibble_2, nibble_3, nibble_4);\
                return fn(vm, nnn);\
            }\
        } while (0);

    #define CHECK_OPCODE_A_B_C_D(a, b, c, d, fn) \
        do {\
            if (nibble_1 == a && nibble_2 == b && nibble_3 == c && nibble_4 == d)\
                return fn(vm);\
        } while (0);

     #define CHECK_OPCODE_A_X_KK(a, fn) \
        do {\
            if (nibble_1 == a) {\
                const uint4_t x = nibble_2;\
                const uint8_t kk = NIBBLES_TO_BYTE(nibble_3, nibble_4);\
                return fn(vm, x, kk);\
            }\
        } while (0);

    #define CHECK_OPCODE_A_X_Y_B(a, b, fn) \
        do {\
            if (nibble_1 == a && nibble_4 == b) {\
                const uint4_t x = nibble_2;\
                const uint4_t y = nibble_3;\
                return fn(vm, x, y);\
            }\
        } while (0);

    #define CHECK_OPCODE_A_X_Y_N(a, fn) \
        do {\
            if (nibble_1 == a) {\
                const uint4_t x = nibble_2;\
                const uint4_t y = nibble_3;\
                uint4_t n = nibble_4;\
                return fn(vm, x, y, n);\
            }\
        } while (0);

    #define CHECK_OPCODE_A_X_B_C(a, b, c, fn) \
        do {\
            if (nibble_1 == a && nibble_3 == b && nibble_4 == c) {\
                const uint4_t x = nibble_2;\
                return fn(vm, x);\
            }\
        } while (0);

    //.. Note: list opcodes from least variables to most variables to prevent
    //   'collisions'.
    CHECK_OPCODE_A_B_C_D(0, 0, 0xE, 0, instruction_cls);
    CHECK_OPCODE_A_B_C_D(0, 0, 0xE, 0xE, instruction_ret);
    CHECK_OPCODE_A_X_B_C(0xE, 9, 0xE, instruction_skp);
    CHECK_OPCODE_A_X_B_C(0xE, 0xA, 1, instruction_sknp);
    CHECK_OPCODE_A_X_B_C(0xF, 0, 7, instruction_ld_vx_dt);
    CHECK_OPCODE_A_X_B_C(0xF, 0, 0xA, instruction_ld_vx_k);
    CHECK_OPCODE_A_X_B_C(0xF, 1, 5, instruction_ld_dt_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 1, 8, instruction_ld_st_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 1, 0xE, instruction_add_i_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 2, 9, instruction_ld_f_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 3, 3, instruction_ld_b_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 5, 5, instruction_ld_i_vx);
    CHECK_OPCODE_A_X_B_C(0xF, 6, 5, instructon_ld_vx_i);
    CHECK_OPCODE_A_X_Y_B(5, 0, instruction_se_vx_vy);
    CHECK_OPCODE_A_X_Y_B(8, 0, instruction_ld_vx_vy);
    CHECK_OPCODE_A_X_Y_B(8, 1, instruction_or);
    CHECK_OPCODE_A_X_Y_B(8, 2, instruction_and);
    CHECK_OPCODE_A_X_Y_B(8, 3, instruction_xor);
    CHECK_OPCODE_A_X_Y_B(8, 4, instruction_add_vx_vy);
    CHECK_OPCODE_A_X_Y_B(8, 5, instruction_sub);
    CHECK_OPCODE_A_X_Y_B(8, 6, instruction_shr);
    CHECK_OPCODE_A_X_Y_B(8, 7, instruction_subn);
    CHECK_OPCODE_A_X_Y_B(8, 0xE, instruction_shl);
    CHECK_OPCODE_A_X_Y_B(9, 0, instruction_sne_vx_vy);
    CHECK_OPCODE_A_X_KK(0xC, instruction_rnd);
    CHECK_OPCODE_A_X_KK(3, instruction_se_vx_byte);
    CHECK_OPCODE_A_X_KK(4, instruction_sne_vx_byte);
    CHECK_OPCODE_A_X_KK(6, instruction_ld_vx_byte);
    CHECK_OPCODE_A_X_KK(7, instruction_add_vx_byte);
    CHECK_OPCODE_A_X_Y_N(0xD, instruction_drw);
    CHECK_OPCODE_A_NNN(1, instruction_jp_addr);
    CHECK_OPCODE_A_NNN(2, instruction_call);
    CHECK_OPCODE_A_NNN(0, instruction_sys);
    CHECK_OPCODE_A_NNN(0xB, instruction_jp_v0_addr);
    CHECK_OPCODE_A_NNN(0xA, instruction_ld_i_addr);

    return E_VM_UNKNOWN_UPCODE;
}

static uint16_t
current_opcode(struct VM vm)
{
    assert(vm.program_counter + 1 <= MEMORY_SIZE);

    return (vm.memory[vm.program_counter] << 8) + vm.memory[vm.program_counter + 1];
}

enum Error
vm_next(struct VM* vm, bool* quit_flag)
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            *quit_flag = true;
            return E_OK;
        }
    }

    //.. Ring system bell when sound timer 'rings'
    if (vm->sound_timer != 0 &&
        vm->sound_timer_last_update + vm->sound_timer >= time(NULL)
    ) {
        io_beep();
        vm->sound_timer = 0;
    }
        
    return execute_opcode(vm, current_opcode(*vm));
}

enum Error
vm_insert_rom(struct VM* vm, const char* file_path)
{
    FILE* file = fopen(file_path, "rb");
    if (file == NULL)
        return E_COULDNT_OPEN_FILE;

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size > (MEMORY_SIZE - 0x200)) {
        fclose(file);
        return E_VM_OUT_OF_MEMORY;
    }

    if (!fread(&vm->memory[0x200], sizeof(uint8_t), file_size, file)) {
        fclose(file);
        return E_COULDNT_READ_FILE;
    }

    fclose(file);
    return E_OK;
}

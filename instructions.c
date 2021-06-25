#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TODO() \
    fprintf(stderr, "TODO: %s() not yet implemented in %s\n", __func__, __FILE__);\
    exit(EXIT_FAILURE);

#define NEXT_INSTRUCTION (vm->program_counter += 2)
#define SKIP_INSTRUCTION (vm->program_counter += 2 * 2)

enum Error
instruction_sys(struct VM* vm, uint12_t nnn)
{
    //.. Not implemented.
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_cls(struct VM* vm)
{
    io_clear_display(&vm->io);
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ret(struct VM* vm)
{
    enum Error err = chip8stack_pop(&vm->stack, &vm->program_counter);
    if (err != E_OK)
        return err;

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_jp_addr(struct VM* vm, uint12_t nnn)
{
    vm->program_counter = nnn;
    return E_OK;
}

enum Error
instruction_call(struct VM* vm, uint12_t nnn)
{
    enum Error err = chip8stack_push(&vm->stack, vm->program_counter);
    if (err != E_OK)
        return err;

    vm->program_counter = nnn;
    return E_OK;
}

enum Error
instruction_se_vx_byte(struct VM* vm, uint4_t x, uint8_t kk)
{
    if (vm->data_registers[x] == kk)
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_sne_vx_byte(struct VM* vm, uint4_t x, uint8_t kk)
{
    if (vm->data_registers[x] != kk)
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_se_vx_vy(struct VM* vm, uint4_t x, uint4_t y)
{
    if (vm->data_registers[x] == vm->data_registers[y])
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_ld_vx_byte(struct VM* vm, uint4_t x, uint8_t kk)
{
    vm->data_registers[x] = kk;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_add_vx_byte(struct VM* vm, uint4_t x, uint8_t kk)
{
    vm->data_registers[x] += kk;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_vx_vy(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[x] = vm->data_registers[y];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_or(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[x] |= vm->data_registers[y];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_and(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[x] &= vm->data_registers[y];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_xor(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[x] ^= vm->data_registers[y];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_add_vx_vy(struct VM* vm, uint4_t x, uint4_t y)
{
    const uint16_t result = vm->data_registers[x] + vm->data_registers[y];
    //.. Set carry flag depending on whether result is greater than 8-bits
    vm->data_registers[VF] = result > 0xFF;
    //.. Only store lowest 8 bits of result
    vm->data_registers[x] = result & 0xFF;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_sub(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[VF] = (vm->data_registers[x] > vm->data_registers[y]);
    vm->data_registers[x] -= vm->data_registers[y];

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_shr(struct VM* vm, uint4_t x, uint4_t y)
{
    //.. Set VF to the least-significant bit
    vm->data_registers[VF] = vm->data_registers[x] & 1;
    vm->data_registers[x] = vm->data_registers[x] >> 1;

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_subn(struct VM* vm, uint4_t x, uint4_t y)
{
    vm->data_registers[VF] = vm->data_registers[y] > vm->data_registers[x];
    vm->data_registers[x] = vm->data_registers[y] - vm->data_registers[x];

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_shl(struct VM* vm, uint4_t x, uint4_t y)
{
    //.. Set VF to the most-significant bit
    vm->data_registers[VF] = (vm->data_registers[x] & (1 << 7)) != 0;
    vm->data_registers[x] = vm->data_registers[x] << 1;

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_sne_vx_vy(struct VM* vm, uint4_t x, uint4_t y)
{
    if (vm->data_registers[x] != vm->data_registers[y])
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_ld_i_addr(struct VM* vm, uint12_t nnn)
{
    vm->address_register = nnn;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_jp_v0_addr(struct VM* vm, uint12_t nnn)
{
    vm->program_counter = vm->data_registers[0] + nnn;
    return E_OK;
}

enum Error
instruction_rnd(struct VM* vm, uint4_t x, uint8_t kk)
{
    vm->data_registers[x] = (rand() % 255) & kk;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_drw(struct VM* vm, uint4_t x, uint4_t y, uint4_t n)
{
    const uint8_t Vx = vm->data_registers[x] % DISPLAY_WIDTH;
    const uint8_t Vy = vm->data_registers[y] % DISPLAY_HEIGHT;
    vm->data_registers[VF] = 0;

    for (int height = 0; height < n; height++) {
        const uint8_t sprite_byte = vm->memory[vm->address_register + height];

        for (int bit_n = 0; bit_n < 8; bit_n++) {
            const bool sprite_bit = (sprite_byte >> (7 - bit_n)) & 1;

            if (sprite_bit) {
                if (vm->io.pixel_map[Vy + height][Vx + bit_n]) {
                    vm->io.pixel_map[Vy + height][Vx + bit_n] = false;
                    vm->data_registers[VF] = 1;
                } else {
                    vm->io.pixel_map[Vy + height][Vx + bit_n] = true;
                }
            }
       }
    }

    enum Error err = io_update_display(&vm->io);
    if (err != E_OK)
        return err;

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_skp(struct VM* vm, uint4_t x)
{
    if (io_is_key_pressed(vm->data_registers[x]))
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_sknp(struct VM* vm, uint4_t x)
{
    if (!io_is_key_pressed(vm->data_registers[x]))
        SKIP_INSTRUCTION;
    else
        NEXT_INSTRUCTION;

    return E_OK;
}

enum Error
instruction_ld_vx_dt(struct VM* vm, uint4_t x)
{
    //.. Determine the value for the delay timer by comparing the UNIX
    //   timestamp from setting the delay timer to the UNIX timestamp of, now,
    //   reading the delay timer.
    const unsigned long sec_since_delay_timer_update =
        time(NULL) - vm->delay_timer_last_update;

    if (sec_since_delay_timer_update > vm->delay_timer)
        vm->delay_timer = 0;
    else
        vm->delay_timer = sec_since_delay_timer_update;

    vm->data_registers[x] = vm->delay_timer;
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_vx_k(struct VM* vm, uint4_t x)
{
    int8_t key_value = io_pressed_key();
    //.. If no key was pressed, NEXT_INSTRUCTION won't be called, in which case
    //   this instruction will be repeatedly called.
    if (key_value >= 0) {
        vm->data_registers[x] = key_value;
        NEXT_INSTRUCTION;
    }
    return E_OK;
}

enum Error
instruction_ld_dt_vx(struct VM* vm, uint4_t x)
{
    vm->delay_timer = vm->data_registers[x];
    vm->delay_timer_last_update = time(NULL);
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_st_vx(struct VM* vm, uint4_t x)
{
    vm->sound_timer = vm->data_registers[x];
    vm->sound_timer_last_update = time(NULL);
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_add_i_vx(struct VM* vm, uint4_t x)
{
    vm->address_register += vm->data_registers[x];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_f_vx(struct VM* vm, uint4_t x)
{
    vm->address_register = FONT_START + 5 * vm->data_registers[x];
    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_b_vx(struct VM* vm, uint4_t x)
{
    const uint8_t Vx = vm->data_registers[x];
    const uint8_t hundreds_digit = Vx / 100;
    const uint8_t tens_digit = (Vx / 10) % 10;
    const uint8_t ones_digit = (Vx % 100) % 10;

    vm->memory[vm->address_register] = hundreds_digit;
    vm->memory[vm->address_register + 1] = tens_digit;
    vm->memory[vm->address_register + 2] = ones_digit;

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instruction_ld_i_vx(struct VM* vm, uint4_t x)
{
    for (uint8_t i = 0; i <= x; i++)
        vm->memory[vm->address_register+i] = vm->data_registers[i];

    NEXT_INSTRUCTION;
    return E_OK;
}

enum Error
instructon_ld_vx_i(struct VM* vm, uint4_t x)
{
    for (int i = 0; i <= x; i++)
        vm->data_registers[i] = vm->memory[vm->address_register+i];
    NEXT_INSTRUCTION;
    return E_OK;
}


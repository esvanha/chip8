#ifndef VM_H_
#define VM_H_

#include "error.h"
#include "io.h"

#include <stdint.h>
#include <stdbool.h>

enum Register {
    V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF
};
#define REGISTERS_SIZE 16
#define MEMORY_SIZE    0xFFF
#define STACK_SIZE     16
#define PROGRAM_START  0x200
#define FONT_START     0x0

struct Chip8Stack {
    uint16_t contents[STACK_SIZE];
    uint8_t length;
};
enum Error chip8stack_push(struct Chip8Stack*, uint16_t);
enum Error chip8stack_pop(struct Chip8Stack*, uint16_t*);

struct VM {
    uint8_t data_registers[REGISTERS_SIZE];
    uint16_t address_register;
    uint16_t program_counter;
    struct Chip8Stack stack;
    struct IO io;

    uint8_t delay_timer;
    unsigned long delay_timer_last_update;
    uint8_t sound_timer;
    unsigned long sound_timer_last_update;

    uint8_t memory[MEMORY_SIZE];
};

enum Error vm_new(struct VM*);
enum Error vm_insert_instruction(struct VM*, int16_t);
void       vm_print_debug(struct VM);
enum Error vm_next(struct VM*, bool*);
enum Error vm_insert_rom(struct VM*, const char*);
void       vm_quit(struct VM*);

#endif

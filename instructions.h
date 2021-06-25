#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "error.h"
#include "vm.h"

#include <stdint.h>
//.. For clarity when addressing nibbles and 12-bit values
typedef uint8_t uint4_t;
typedef uint16_t uint12_t;

enum Error instruction_sys(struct VM*, uint12_t nnn);
enum Error instruction_cls(struct VM*);
enum Error instruction_ret(struct VM*);
enum Error instruction_jp_addr(struct VM*, uint12_t nnn);
enum Error instruction_call(struct VM*, uint12_t nnn);
enum Error instruction_se_vx_byte(struct VM*, uint4_t x, uint8_t kk);
enum Error instruction_sne_vx_byte(struct VM*, uint4_t x, uint8_t kk);
enum Error instruction_se_vx_vy(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_ld_vx_byte(struct VM*, uint4_t x, uint8_t kk);
enum Error instruction_add_vx_byte(struct VM*, uint4_t x, uint8_t kk);
enum Error instruction_ld_vx_vy(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_or(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_and(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_xor(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_add_vx_vy(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_sub(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_shr(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_subn(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_shl(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_sne_vx_vy(struct VM*, uint4_t x, uint4_t y);
enum Error instruction_ld_i_addr(struct VM*, uint12_t nnn);
enum Error instruction_jp_v0_addr(struct VM*, uint12_t nnn);
enum Error instruction_rnd(struct VM*, uint4_t x, uint8_t kk);
enum Error instruction_drw(struct VM*, uint4_t x, uint4_t y, uint4_t n);
enum Error instruction_skp(struct VM*, uint4_t x);
enum Error instruction_sknp(struct VM*, uint4_t x);
enum Error instruction_ld_vx_dt(struct VM*, uint4_t x);
enum Error instruction_ld_vx_k(struct VM*, uint4_t x);
enum Error instruction_ld_dt_vx(struct VM*, uint4_t x);
enum Error instruction_ld_st_vx(struct VM*, uint4_t x);
enum Error instruction_add_i_vx(struct VM*, uint4_t x);
enum Error instruction_ld_f_vx(struct VM*, uint4_t x);
enum Error instruction_ld_b_vx(struct VM*, uint4_t x);
enum Error instruction_ld_i_vx(struct VM*, uint4_t x);
enum Error instructon_ld_vx_i(struct VM*, uint4_t x);

#endif

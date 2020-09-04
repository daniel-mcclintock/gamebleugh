#ifndef CPU_H
#define CPU_H

#include "ops.h"
#include <stdlib.h>

// flag masks
#define ZERO_FLAG 0x80
#define SUBTRACT_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10

// argument flags affected
#define ZZ 0x80
#define Z0 0x08
#define Z1 0x88

#define NN 0x40
#define N0 0x04
#define N1 0x44

#define HH 0x20
#define H0 0x02
#define H1 0x22

#define CC 0x10
#define C0 0x01
#define C1 0x11

#define PREFIX_BYTE_OFFSET 0x100

typedef struct {
  /* 8-bit registers */
  // note the order, pair pointers will align with their first element
  u_int8_t A, C, B, E, D, L, H;

  /* 16-bit registers */
  u_int16_t PC;
  u_int16_t SP;

  /*
    There is an additional 8-bit register used for flags.
    NOTE: Lower 4 bits are not used and always zero.
    Eg; 0b00000000
          ^||||||| < ZERO_FLAG
           ^|||||| < SUBTRACT_FLAG
            ^||||| < HALF_CARRY_FLAG
             ^|||| < CARRY_FLAG
              ^^^^ < Not Used
  */
  u_int8_t FLAGS;

} Registers;

typedef struct {
  Registers registers;

} CPU;

void apply_flags(Op *op);
void set_flag(u_int16_t mask, u_int8_t value);
u_int8_t get_flag(u_int8_t mask);

extern CPU cpu;

#endif

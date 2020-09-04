#include "cpu.h"
#include <stdio.h>

CPU cpu;

/* utilities */
void apply_flags(Op *op) {
  if ((op->flags & Z0) == Z0) {
    set_flag(ZERO_FLAG, 0);
  } else if ((op->flags & Z1) == Z1) {
    set_flag(ZERO_FLAG, 1);
  }

  if ((op->flags & N0) == N0) {
    set_flag(SUBTRACT_FLAG, 0);
  } else if ((op->flags & N1) == N1) {
    set_flag(SUBTRACT_FLAG, 1);
  }

  if ((op->flags & H0) == H0) {
    set_flag(HALF_CARRY_FLAG, 0);
  } else if ((op->flags & H1) == H1) {
    set_flag(HALF_CARRY_FLAG, 1);
  }

  if ((op->flags & C0) == C0) {
    set_flag(CARRY_FLAG, 0);
  } else if ((op->flags & C1) == C1) {
    set_flag(CARRY_FLAG, 1);
  }
}

void set_flag(u_int16_t mask, u_int8_t value) {
  if (value) {
    cpu.registers.FLAGS |= mask;
  } else {
    cpu.registers.FLAGS &= ~mask;
  }
}

u_int8_t get_flag(u_int8_t mask) {
  return (cpu.registers.FLAGS & mask) == mask;
}

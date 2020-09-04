#include "cpu.h"
#include "decode.h"
#include "mmu.h"
#include "ops.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

int apply_flag(u_int8_t flag, u_int8_t flags_affected) {
  return 0;
}

int8_t uint8_to_int8(u_int8_t *value) {
  /* Convert unsigned byte to signed byte */
  int8_t signed_value = *value;

  if (signed_value > 127) {
    signed_value = -((~signed_value + 1) & 255);
  }

  return signed_value;
}

u_int8_t *NOT_IMPLEMENTED(u_int8_t *cycles, Arg *arg1, Arg *arg2) {
  printf("NOT_IMPLEMENTED\n");
  exit(1);
}

u_int8_t *NOP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *LD(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Load arg2 into arg1 */
  u_int8_t *data = arg2->fetch();
  arg1->write(data);

  /* increment / decrement arguments, only used here */
  // TODO: Urgh, this is shit. why am i doing this here?
  if (strcmp(arg1->name, "(HL-)") == 0) {
    cpu.registers.L--;

    if (cpu.registers.L == 255) {
      cpu.registers.H--;
    }
    set_flag(ZERO_FLAG, cpu.registers.H + cpu.registers.L == 0);
  } else if (strcmp(arg1->name, "(HL+)") == 0) {
    cpu.registers.L++;

    if (cpu.registers.L == 0) {
      cpu.registers.H++;
    }
  }

  return cycles_t;
}

u_int8_t *INC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Increment arg1 */
  u_int8_t *data = arg1->fetch();

  if (arg1->size == 2) {
    // increment lower byte
    (*data)++;

    // did we roll over?
    if (*data == 0) {
      // increment upper byte
      data++;
      (*data)++;
      set_flag(ZERO_FLAG, *data == 0);
    }
    return cycles_t;
  }

  // single byte argument
  (*data)++;
  set_flag(ZERO_FLAG, *data == 0);

  return cycles_t;
}

u_int8_t *DEC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Decrement arg1 */
  u_int8_t *data = arg1->fetch();

  if (arg1->size == 2) {
    // increment lower byte
    data++;
    (*data)--;

    // did we roll over?
    if (*data == 0) {
      // increment upper byte
      data--;
      (*data)--;

      set_flag(ZERO_FLAG, *data == 0);
    }
    return cycles_t;
  }

  // single byte argument
  (*data)--;
  set_flag(ZERO_FLAG, *data == 0);

  return cycles_t;
}

u_int8_t *RLCA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *ADD(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *RRCA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *STOP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *JR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Relative Jump */
  if (arg2 != NULL) {
    /*
     * We have 2 arguments, this is a conditional jump.
     * arg1 == condition
     * arg2 == value
     * */
    if (*arg1->fetch()) {
      if (strcmp(arg2->name, "r8") == 0) {
        cpu.registers.PC += uint8_to_int8(arg2->fetch());
      } else {
        cpu.registers.PC += *arg2->fetch();
      }

      return cycles_t;
    }

    return cycles_f;
  }
  /*
   * Single argument, .. just jump
   * */

  if (strcmp(arg1->name, "r8") == 0) {
    cpu.registers.PC += uint8_to_int8(arg1->fetch());
  } else {
    cpu.registers.PC += *arg1->fetch();
  }

  return cycles_t;
}

u_int8_t *RRA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *DAA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *CPL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *SCF(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *CCF(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *HALT(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *ADC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *SUB(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Subtract from A */
  u_int8_t *data = arg1->fetch();
  set_flag(SUBTRACT_FLAG, cpu.registers.A < *data);
  u_int8_t a = cpu.registers.A - *data;

  // H - Set if no borrow from bit 4
  if (cpu.registers.A ^ *data ^ a & 0x10) {
    set_flag(HALF_CARRY_FLAG, 1);
  }

  set_flag(ZERO_FLAG, a == 0);

  // C - Set for no borrow, set if A < n
  set_flag(CARRY_FLAG, cpu.registers.A < *data);
  cpu.registers.A = a;

  return cycles_t;
}

u_int8_t *SBC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *AND(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *XOR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* exclusive or arg1 against A */
  u_int8_t *data = arg1->fetch();
  cpu.registers.A ^= *data;
  set_flag(ZERO_FLAG, !cpu.registers.A);
  return cycles_t;
}

u_int8_t *OR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *CP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Compare Arg1 against A, always 1 argument. always 8-bit. */
  // Not actually a comparison
  u_int8_t *data = arg1->fetch();
  u_int8_t a = cpu.registers.A;
  a -= *data;
  a &= 255;

  // Flags
  set_flag(ZERO_FLAG, a == 0);

  // H - Set if no borrow from bit-4
  u_int8_t hc = (cpu.registers.A ^ a ^ *data) & 0x10; // Wait isn't 0x10 carry?
  set_flag(HALF_CARRY_FLAG, hc);

  // C - Set if no borrow(set if A < n)
  set_flag(CARRY_FLAG, cpu.registers.A < *data);

  return cycles_t;
}

u_int8_t *RET(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* Return from subroutine */
  cpu.registers.PC = *mmu_fetch(&cpu.registers.SP) << 8;
  cpu.registers.SP++;
  cpu.registers.PC += *mmu_fetch(&cpu.registers.SP) & 255;
  cpu.registers.SP++;

  return cycles_t;
}

u_int8_t *POP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* pop from stack into arg1, always operates on registers pairs */
  u_int8_t *stack = mmu_fetch(&cpu.registers.SP);

  // Special case where we have to manually re-order the bytes
  u_int16_t stack16 = *(stack) << 8;
  stack16 += *(++stack);

  arg1->write((u_int8_t *)&stack16);

  cpu.registers.SP += 2;

  return cycles_t;
}

u_int8_t *JP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *CALL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  if (arg2 != NULL) {
    /*
     * We have 2 arguments, this is a conditional call.
     * arg1 == condition
     * arg2 == value
     * */
    return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
  }

  // Store current PC to SP addr
  cpu.registers.SP -= 2;

  // Use current PC + size of CALL op as the PC to RET at end of CALL
  u_int16_t target_pc = cpu.registers.PC + 2;
  u_int8_t pc = target_pc >> 8;
  u_int16_t sp = cpu.registers.SP;

  mmu_write(&sp, &pc);
  sp++;
  pc = target_pc & 255;
  mmu_write(&sp, &pc);

  // less op length (2), we could improve this by not always adding OP length
  // in gamebleugh.c
  cpu.registers.PC = (*(arg1->fetch() + 1) << 8) + *arg1->fetch() - 2;

  return cycles_t;
}

u_int8_t *PUSH(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  /* push arg1 onto stack, always operates on registers pairs */
  u_int8_t *data = arg1->fetch();

  cpu.registers.SP -= 1;
  mmu_write(&cpu.registers.SP, data);

  data++;
  cpu.registers.SP -= 1;
  mmu_write(&cpu.registers.SP, data);

  return cycles_t;
}

u_int8_t *RST(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *RETI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *DI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

u_int8_t *EI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  return NOT_IMPLEMENTED(cycles_t, arg1, arg2);
}

/* PREFIX OPS */
u_int8_t *RL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  u_int8_t *data = arg1->fetch();

  u_int8_t carry_in = get_flag(CARRY_FLAG);
  set_flag(CARRY_FLAG, (*data & 0x80) == 0x80);

  *data = ((*data << 1) + carry_in) & 255;
  arg1->write(data);

  return cycles_t;
}

u_int8_t *BIT(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2) {
  u_int8_t mask = 1 << (*arg1->fetch());
  set_flag(ZERO_FLAG, (*arg2->fetch() & mask) != mask);

  return cycles_t;
}

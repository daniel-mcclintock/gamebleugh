#ifndef OPS_H
#define OPS_H

#include <stdio.h>
#include <sys/types.h>

#define PREFIX_BYTE 0xCB

typedef struct {
  const char *name;

  u_int8_t size;
  u_int8_t *(*fetch)();
  void (*write)(u_int8_t *value);
} Arg;

typedef struct {
  const char *name;
  u_int8_t *(*op)(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);

  Arg *arg1;
  Arg *arg2;
  u_int8_t length;
  u_int8_t cycles_t;
  u_int8_t cycles_f;
  u_int8_t flags;
} Op;

u_int8_t *NOT_IMPLEMENTED(u_int8_t *cycles, Arg *arg1, Arg *arg2);
u_int8_t *NOP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *LD(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *INC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *DEC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RLCA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *ADD(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RRCA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *STOP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RLA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *JR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RRA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *DAA(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *CPL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *SCF(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *CCF(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *HALT(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *ADC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *SUB(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *SBC(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *AND(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *XOR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *OR(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *CP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RET(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *POP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *JP(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *CALL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *PUSH(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RST(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *RETI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *DI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *EI(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);

/* PREFIX OPS */
u_int8_t *RL(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);
u_int8_t *BIT(u_int8_t *cycles_t, u_int8_t *cycles_f, Arg *arg1, Arg *arg2);

#endif

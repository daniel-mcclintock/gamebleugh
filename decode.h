#ifndef DECODE_H
#define DECODE_H

#include "cpu.h"
#include "ops.h"
#include <sys/types.h>

/* cpu registers 8-bit */
u_int8_t *fetch_A();
void write_A(u_int8_t *value);
static Arg *A;

u_int8_t *fetch_B();
void write_B(u_int8_t *value);
static Arg *B;

u_int8_t *fetch_C();
void write_C(u_int8_t *value);
static Arg *C;

u_int8_t *fetch_D();
void write_D(u_int8_t *value);
static Arg *D;

u_int8_t *fetch_E();
void write_E(u_int8_t *value);
static Arg *E;

u_int8_t *fetch_H();
void write_H(u_int8_t *value);
static Arg *H;

u_int8_t *fetch_L();
void write_L(u_int8_t *value);
static Arg *L;

/* cpu registers 16-bit */
u_int8_t *fetch_BC();
void write_BC(u_int8_t *value);
static Arg *BC;

u_int8_t *fetch_DE();
void write_DE(u_int8_t *value);
static Arg *DE;

u_int8_t *fetch_HL();
void write_HL(u_int8_t *value);
static Arg *HL;

u_int8_t *fetch_a_BC();
void write_a_BC(u_int8_t *value);
static Arg *a_BC;

u_int8_t *fetch_a_DE();
void write_a_DE(u_int8_t *value);
static Arg *a_DE;

u_int8_t *fetch_a_HL();
void write_a_HL(u_int8_t *value);
static Arg *a_HL;

// u_int8_t *fetch_a_HL_m();
// void write_a_HL_m(u_int8_t *value);
static Arg *a_HL_m;

// u_int8_t *fetch_a_HL_p();
// void write_a_HL_p(u_int8_t *value);
static Arg *a_HL_p;

/* immediate reads */
u_int8_t *fetch_d16();
static Arg *d16;

u_int8_t *fetch_d8();
static Arg *d8;

u_int8_t *fetch_r8();
static Arg *r8;

/* flags */
u_int8_t *fetch_f_NC();
static Arg *f_NC;

u_int8_t *fetch_f_NZ();
static Arg *f_NZ;

u_int8_t *fetch_f_Z();
static Arg *f_Z;

/* other */
u_int8_t *fetch_SP();
void write_SP(u_int8_t *value);
static Arg *SP;

u_int8_t *fetch_SP_r8();
static Arg *SP_r8;

u_int8_t *fetch_a_FF00_C();
void write_a_FF00_C(u_int8_t *value);
static Arg *a_FF00_C;

u_int8_t *fetch_a_FF00_a8();
void write_a_FF00_a8(u_int8_t *value);
static Arg *a_FF00_a8;

u_int8_t *fetch_a16();
static Arg *a16;

u_int8_t *fetch_a_a16();
void write_a_a16(u_int8_t *value);
static Arg *a_a16;

/* INT ARGS */
// No writes here

u_int8_t *fetch_i0();
static Arg *i0;

u_int8_t *fetch_i1();
static Arg *i1;

u_int8_t *fetch_i2();
static Arg *i2;

u_int8_t *fetch_i3();
static Arg *i3;

u_int8_t *fetch_i4();
static Arg *i4;

u_int8_t *fetch_i5();
static Arg *i5;

u_int8_t *fetch_i6();
static Arg *i6;

u_int8_t *fetch_i7();
static Arg *i7;

Op *get_opmap();

#endif

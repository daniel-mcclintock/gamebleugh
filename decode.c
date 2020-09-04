#include "cpu.h"
#include "decode.h"
#include "mmu.h"

/*
  Initially it wasn't so bad having these Args encapsulated into their own
  little handler structs, but now its just getting messy and it makes writing
  the individual Ops more complicated. I think this should be revised.
  Theres way to much cognitive overhead for each Op and each Argument type
*/

/* cpu registers 8-bit */
u_int8_t *fetch_A() { return &cpu.registers.A; }
void write_A(u_int8_t *value) { cpu.registers.A = *value & 255; }
static Arg *A = &(Arg){"A", 1, fetch_A, write_A};

u_int8_t *fetch_B() { return &cpu.registers.B; }
void write_B(u_int8_t *value) { cpu.registers.B = *value & 255; }
static Arg *B = &(Arg){"B", 1, fetch_B, write_B};

u_int8_t *fetch_C() { return &cpu.registers.C; }
void write_C(u_int8_t *value) { cpu.registers.C = *value & 255; }
static Arg *C = &(Arg){"C", 1, fetch_C, write_C};

u_int8_t *fetch_D() { return &cpu.registers.D; }
void write_D(u_int8_t *value) { cpu.registers.D = *value & 255; }
static Arg *D = &(Arg){"D", 1, fetch_D, write_D};

u_int8_t *fetch_E() { return &cpu.registers.E; }
void write_E(u_int8_t *value) { cpu.registers.E = *value & 255; }
static Arg *E = &(Arg){"E", 1, fetch_E, write_E};

u_int8_t *fetch_H() { return &cpu.registers.H; }
void write_H(u_int8_t *value) { cpu.registers.H = *value & 255; }
static Arg *H = &(Arg){"H", 1, fetch_H, write_H};

u_int8_t *fetch_L() { return &cpu.registers.L; }
void write_L(u_int8_t *value) { cpu.registers.L = *value & 255; }
static Arg *L = &(Arg){"L", 1, fetch_L, write_L};

/* cpu registers 16-bit
 * NOTE: These registers shadow the 8-bit tail-register.
 * */
u_int8_t *fetch_BC() { return &cpu.registers.C; }
void write_BC(u_int8_t *value) {
  cpu.registers.C = *value & 255;
  cpu.registers.B = *(++value) & 255;
}
static Arg *BC = &(Arg){"BC", 2, fetch_BC, write_BC};

u_int8_t *fetch_DE() { return &cpu.registers.E; }
void write_DE(u_int8_t *value) {
  cpu.registers.E = *value & 255;
  cpu.registers.D = *(++value) & 255;
}
static Arg *DE = &(Arg){"DE", 2, fetch_DE, write_DE};

u_int8_t *fetch_HL() { return &cpu.registers.L; }
void write_HL(u_int8_t *value) {
  cpu.registers.L = *value & 255;
  cpu.registers.H = *(++value) & 255;
}
static Arg *HL = &(Arg){"HL", 2, fetch_HL, write_HL};

/* address of 16-bit register */
u_int8_t *fetch_a_BC() {
  u_int16_t addr = cpu.registers.B << 8;
  addr += cpu.registers.C;
  return mmu_fetch(&addr);
}

void write_a_BC(u_int8_t *value) {
  u_int16_t addr = cpu.registers.B << 8;
  addr += cpu.registers.C;
  mmu_write(&addr, value);
}
static Arg *a_BC = &(Arg){"(BC)", 1, fetch_a_BC, write_a_BC};

u_int8_t *fetch_a_DE() {
  u_int16_t addr = cpu.registers.D << 8;
  addr += cpu.registers.E;
  return mmu_fetch(&addr);
}
void write_a_DE(u_int8_t *value) {
  u_int16_t addr = cpu.registers.D << 8;
  addr += cpu.registers.E;
  mmu_write(&addr, value);
}
static Arg *a_DE = &(Arg){"(DE)", 1, fetch_a_DE, write_a_DE};

u_int8_t *fetch_a_HL() {
  u_int16_t addr = cpu.registers.H << 8;
  addr += cpu.registers.L;
  return mmu_fetch(&addr);
}
void write_a_HL(u_int8_t *value) {
  u_int16_t addr = cpu.registers.H << 8;
  addr += cpu.registers.L;
  mmu_write(&addr, value);
}
static Arg *a_HL = &(Arg){"(HL)", 1, fetch_a_HL, write_a_HL};
static Arg *a_HL_m = &(Arg){"(HL-)", 1, fetch_a_HL, write_a_HL};
static Arg *a_HL_p = &(Arg){"(HL+)", 1, fetch_a_HL, write_a_HL};

/* immediate reads */
u_int8_t *fetch_d16() { return mmu_fetch(&cpu.registers.PC); }
static Arg *d16 = &(Arg){"d16", 2, fetch_d16, NULL};

u_int8_t *fetch_d8() { return mmu_fetch(&cpu.registers.PC); }
static Arg *d8 = &(Arg){"d8", 1, fetch_d8, NULL};

u_int8_t *fetch_r8() {
  // TODO: this sould be signed
  return mmu_fetch(&cpu.registers.PC);
}
static Arg *r8 = &(Arg){"r8", 1, fetch_r8, NULL};

/* flags */
u_int8_t *fetch_f_NC() {
  // TODO: use get_flag
  static u_int8_t flag = 0;
  flag = (cpu.registers.FLAGS & CARRY_FLAG) ? 1 : 0;
  return &flag;
}
static Arg *f_NC = &(Arg){"f_NC", 1, fetch_f_NC, NULL};

u_int8_t *fetch_f_NZ() {
  // TODO: use get_flag
  static u_int8_t flag = 0;
  flag = (cpu.registers.FLAGS & ZERO_FLAG) ? 0 : 1;
  return &flag;
}
static Arg *f_NZ = &(Arg){"f_NZ", 1, fetch_f_NZ, NULL};

u_int8_t *fetch_f_Z() {
  // TODO: use get_flag
  static u_int8_t flag = 0;
  flag = (cpu.registers.FLAGS & ZERO_FLAG) ? 1 : 0;
  return &flag;
}
static Arg *f_Z = &(Arg){"f_Z", 1, fetch_f_Z, NULL};

/* other */
u_int8_t *fetch_SP() { return (u_int8_t *)&cpu.registers.SP; }
void write_SP(u_int8_t *value) {
  cpu.registers.SP = (*value) + (*(value + 1) << 8);
}
static Arg *SP = &(Arg){"SP", 2, fetch_SP, write_SP};

u_int8_t *fetch_SP_r8() {
  static u_int16_t addr = 0x0000;
  addr = cpu.registers.SP + *fetch_r8();
  return (u_int8_t *)&addr;
}
static Arg *SP_r8 = &(Arg){"SP+r8", 2, fetch_SP_r8, NULL};

u_int8_t *fetch_a_FF00_C() {
  u_int16_t addr = 0xFF00 + cpu.registers.C;
  return mmu_fetch(&addr);
}
void write_a_FF00_C(u_int8_t *value) {
  u_int16_t addr = 0xFF00 + cpu.registers.C;
  mmu_write(&addr, value);
}
static Arg *a_FF00_C = &(Arg){"(0xFF00+C)", 1, fetch_a_FF00_C, write_a_FF00_C};

u_int8_t *fetch_a_FF00_a8() {
  u_int16_t addr = 0xFF00 + *mmu_fetch(&cpu.registers.PC);
  return mmu_fetch(&addr);
}

void write_a_FF00_a8(u_int8_t *value) {
  u_int16_t addr = 0xFF00 + *mmu_fetch(&cpu.registers.PC);
  mmu_write(&addr, value);
}
static Arg *a_FF00_a8 =
    &(Arg){"(0xFF00+a8)", 1, fetch_a_FF00_a8, write_a_FF00_a8};

u_int8_t *fetch_a16() { return mmu_fetch(&cpu.registers.PC); }
static Arg *a16 = &(Arg){"a16", 2, fetch_a16, NULL};

u_int8_t *fetch_a_a16() {
  u_int16_t *addr = (u_int16_t *)fetch_a16();
  return mmu_fetch(addr);
}
void write_a_a16(u_int8_t *value) {
  u_int16_t *addr = (u_int16_t *)fetch_a16();
  mmu_write(addr, value);
}
static Arg *a_a16 = &(Arg){"(a16)", 1, fetch_a_a16, write_a_a16};

/* INT ARGS */
// No writes here
u_int8_t *fetch_i0() {
  static u_int8_t i = 0;
  return &i;
}
static Arg *i0 = &(Arg){"0", 1, fetch_i0, NULL};

u_int8_t *fetch_i1() {
  static u_int8_t i = 1;
  return &i;
}
static Arg *i1 = &(Arg){"1", 1, fetch_i1, NULL};

u_int8_t *fetch_i2() {
  static u_int8_t i = 2;
  return &i;
}
static Arg *i2 = &(Arg){"2", 1, fetch_i2, NULL};

u_int8_t *fetch_i3() {
  static u_int8_t i = 3;
  return &i;
}
static Arg *i3 = &(Arg){"3", 1, fetch_i3, NULL};

u_int8_t *fetch_i4() {
  static u_int8_t i = 4;
  return &i;
}
static Arg *i4 = &(Arg){"4", 1, fetch_i4, NULL};

u_int8_t *fetch_i5() {
  static u_int8_t i = 5;
  return &i;
}
static Arg *i5 = &(Arg){"5", 1, fetch_i5, NULL};

u_int8_t *fetch_i6() {
  static u_int8_t i = 6;
  return &i;
}
static Arg *i6 = &(Arg){"6", 1, fetch_i6, NULL};

u_int8_t *fetch_i7() {
  static u_int8_t i = 7;
  return &i;
}
static Arg *i7 = &(Arg){"7", 1, fetch_i7, NULL};

/*
  The CPU has a FLAGS register, only the upper 4 bits are used, the remaining are
  always zero.

  The assembly notation for each op shows 4 possible values to represent the op
  behaviour, with 3 common values defining the same behaviour for each FLAG.

  The common values are:
      `0` - Reset FLAG after instruction
      `1` - Set FLAG after instruction
      `-` - Does not change FLAG

  With a fourth value representing if the corresponding FLAG may changed by `Op`
      `Z` - ZERO_FLAG may be changed based on operation function.
      `N` - SUBTRACT_FLAG may be changed based on operation function.
      `H` - HALF_CARRY_FLAG may be changed based on operation function.
      `C` - CARRY_FLAG may be changed based on operation function.

  We will pass in a 8bit value as an argument to the Op container so we can
  represent this notation.

      0b00000000   FLAGS Argument
      ------------ ZERO_FLAG
        1          Z
        0   1      0
        1   1      1
        0   0      -
      ------------ SUBTRACT_FLAG
         1         N
         0   1     0
         1   1     1
         0   0     -
      ------------ HALF_CARRY_FLAG
          1        H
          0   1    0
          1   1    1
          0   0    -
      ------------ CARRY_FLAG
           1       C
           0   1   0
           1   1   1
           0   0   -

  You can use the following to affect the bits of the FLAGS argument.

  FLAGS = 0x0 (- - - -)
  ZZ = 0x80
  Z0 = 0x08
  Z1 = 0x88

  NN = 0x40
  N0 = 0x04
  N1 = 0x44

  HH = 0x20
  H0 = 0x02
  H1 = 0x22

  CC = 0x10
  C0 = 0x01
  C1 = 0x11

  Eg;
      To use flags (Z N H C):

      FLAGS = ZZ | NN | HH | CC

      To use flags (Z - - C):

      FLAGS = ZZ | CC

      To use flags (0 - H 1):

      FLAGS = Z0 | HH | C1

      etc...
*/
Op *get_opmap() {
  return &(Op[0xFFFF]){
      // NOTE: length is the Op byte length less the byte itself
      // opcode, name, func, arg1, arg2, length, cycles_t, cycles_f, flags
      /* 0x00 */ (Op){"NOP", NOP, NULL, NULL, 0, 4, 4, 0},
      /* 0x01 */ (Op){"LD", LD, BC, d16, 2, 12, 12, 0},       
      /* 0x02 */ (Op){"LD", LD, a_BC, A, 0, 8, 8, 0},        
      /* 0x03 */ (Op){"INC", INC, BC, NULL, 0, 8, 8, 0},    
      /* 0x04 */ (Op){"INC", INC, B, NULL, 0, 4, 4, ZZ | N0 | HH},
      /* 0x05 */ (Op){"DEC", DEC, B, NULL, 0, 4, 4, ZZ | N1 | HH},
      /* 0x06 */ (Op){"LD", LD, B, d8, 1, 8, 8, 0},               
      /* 0x07 */ (Op){"RLCA", RLCA, NULL, NULL, 0, 4, 4, Z0 | N0 | H0 | CC}, 
      /* 0x08 */ (Op){"LD", LD, a16, SP, 2, 20, 20, 0},            
      /* 0x09 */ (Op){"ADD", ADD, HL, BC, 0, 8, 8, N0 | HH | CC},  
      /* 0x0a */ (Op){"LD", LD, A, a_BC, 0, 8, 8, 0},              
      /* 0x0b */ (Op){"DEC", DEC, BC, NULL, 0, 8, 8, 0},           
      /* 0x0c */ (Op){"INC", INC, C, NULL, 0, 4, 4, ZZ | N0 | HH}, 
      /* 0x0d */ (Op){"DEC", DEC, C, NULL, 0, 4, 4, ZZ | N1 | HH}, 
      /* 0x0e */ (Op){"LD", LD, C, d8, 1, 8, 8, 0},                
      /* 0x0f */ (Op){"RRCA", RRCA, NULL, NULL, 0, 4, 4, Z0 | N0 | H0 | CC},
      /* 0x10 */ (Op){"STOP", STOP, NULL, NULL, 1, 4, 4, 0},
      /* 0x11 */ (Op){"LD", LD, DE, d16, 2, 12, 12, 0},               
      /* 0x12 */ (Op){"LD", LD, a_DE, A, 0, 8, 8, 0},                 
      /* 0x13 */ (Op){"INC", INC, DE, NULL, 0, 8, 8, 0},              
      /* 0x14 */ (Op){"INC", INC, D, NULL, 0, 4, 4, ZZ | N0 | HH},    
      /* 0x15 */ (Op){"DEC", DEC, D, NULL, 0, 4, 4, ZZ | N1 | HH},    
      /* 0x16 */ (Op){"LD", LD, D, d8, 1, 8, 8, 0},                   
      /* 0x17 */ (Op){"RL", RL, A, NULL, 0, 4, 4, Z0 | N0 | H0 | CC}, 
      /* 0x18 */ (Op){"JR", JR, r8, NULL, 1, 12, 12, 0},              
      /* 0x19 */ (Op){"ADD", ADD, HL, DE, 0, 8, 8, 0},                
      /* 0x1a */ (Op){"LD", LD, A, a_DE, 0, 8, 8, 0},                 
      /* 0x1b */ (Op){"DEC", DEC, DE, NULL, 0, 8, 8, 0},           
      /* 0x1c */ (Op){"INC", INC, E, NULL, 0, 4, 4, ZZ | N0 | HH}, 
      /* 0x1d */ (Op){"DEC", DEC, E, NULL, 0, 4, 4, ZZ | N1 | HH}, 
      /* 0x1e */ (Op){"LD", LD, E, d8, 1, 8, 8, 0},                
      /* 0x1f */ (Op){"RRA", RRA, NULL, NULL, 0, 4, 4, Z0 | N0 | H0 | CC},
      /* 0x20 */ (Op){"JR", JR, f_NZ, r8, 1, 12, 8, 0},            
      /* 0x21 */ (Op){"LD", LD, HL, d16, 2, 12, 12, 0},            
      /* 0x22 */ (Op){"LD", LD, a_HL_p, A, 0, 8, 8, 0},            
      /* 0x23 */ (Op){"INC", INC, HL, NULL, 0, 8, 8, 0},           
      /* 0x24 */ (Op){"INC", INC, H, NULL, 0, 4, 4, ZZ | N0 | HH}, 
      /* 0x25 */ (Op){"DEC", DEC, H, NULL, 0, 4, 4, ZZ | N1 | HH}, 
      /* 0x26 */ (Op){"LD", LD, H, d8, 1, 8, 8, 0},                
      /* 0x27 */ (Op){"DAA", DAA, NULL, NULL, 0, 4, 4, ZZ | H0 | CC},
      /* 0x28 */ (Op){"JR", JR, f_Z, r8, 1, 12, 8, 0},             
      /* 0x29 */ (Op){"ADD", ADD, HL, HL, 0, 8, 8, N0 | HH | CC},  
      /* 0x2a */ (Op){"LD", LD, A, a_HL_p, 0, 8, 8, 0},            
      /* 0x2b */ (Op){"DEC", DEC, HL, NULL, 0, 8, 8, 0},           
      /* 0x2c */ (Op){"INC", INC, L, NULL, 0, 4, 4, ZZ | N0 | HH}, 
      /* 0x2d */ (Op){"DEC", DEC, L, NULL, 0, 4, 4, ZZ | N1 | HH}, 
      /* 0x2e */ (Op){"LD", LD, L, d8, 1, 8, 8, 0},                
      /* 0x2f */ (Op){"CPL", CPL, NULL, NULL, 0, 4, 4, N1 | H1},
      /* 0x30 */ (Op){"JR", JR, f_NC, r8, 1, 12, 8, 0},            
      /* 0x31 */ (Op){"LD", LD, SP, d16, 2, 12, 12, 0},            
      /* 0x32 */ (Op){"LD", LD, a_HL_m, A, 0, 8, 8, 0},            
      /* 0x33 */ (Op){"INC", INC, SP, NULL, 0, 8, 8, 0},           
      /* 0x34 */ (Op){"INC", INC, a_HL, NULL, 0, 12, 12, ZZ | N0 | HH}, 
      /* 0x35 */ (Op){"DEC", DEC, a_HL, NULL, 0, 12, 12, ZZ | N1 | HH}, 
      /* 0x36 */ (Op){"LD", LD, a_HL, d8, 1, 12, 12, 0},                
      /* 0x37 */ (Op){"SCF", SCF, NULL, NULL, 0, 4, 4, N0 | H0 | C1},
      /* 0x38 */ (Op){"JR", JR, C, r8, 1, 12, 8, 0}, // TODO: fix condition
      /* 0x39 */ (Op){"ADD", ADD, HL, SP, 0, 8, 8, N0 | HH | CC},  
      /* 0x3a */ (Op){"LD", LD, A, a_HL_m, 0, 8, 8, 0},            
      /* 0x3b */ (Op){"DEC", DEC, SP, NULL, 0, 8, 8, 0},           
      /* 0x3c */ (Op){"INC", INC, A, NULL, 0, 4, 4, ZZ | N0 | HH}, 
      /* 0x3d */ (Op){"DEC", DEC, A, NULL, 0, 4, 4, ZZ | N1 | HH}, 
      /* 0x3e */ (Op){"LD", LD, A, d8, 1, 8, 8, 0},                
      /* 0x3f */ (Op){"CCF", CCF, NULL, NULL, 0, 4, 4, N0 | H0 | CC},
      /* 0x40 */ (Op){"LD", LD, B, B, 0, 4, 4, 0},    
      /* 0x41 */ (Op){"LD", LD, B, C, 0, 4, 4, 0},    
      /* 0x42 */ (Op){"LD", LD, B, D, 0, 4, 4, 0},    
      /* 0x43 */ (Op){"LD", LD, B, E, 0, 4, 4, 0},    
      /* 0x44 */ (Op){"LD", LD, B, H, 0, 4, 4, 0},    
      /* 0x45 */ (Op){"LD", LD, B, L, 0, 4, 4, 0},    
      /* 0x46 */ (Op){"LD", LD, B, a_HL, 0, 8, 8, 0}, 
      /* 0x47 */ (Op){"LD", LD, B, A, 0, 4, 4, 0},    
      /* 0x48 */ (Op){"LD", LD, C, B, 0, 4, 4, 0},    
      /* 0x49 */ (Op){"LD", LD, C, C, 0, 4, 4, 0},    
      /* 0x4a */ (Op){"LD", LD, C, D, 0, 4, 4, 0},    
      /* 0x4b */ (Op){"LD", LD, C, E, 0, 4, 4, 0},    
      /* 0x4c */ (Op){"LD", LD, C, H, 0, 4, 4, 0},    
      /* 0x4d */ (Op){"LD", LD, C, L, 0, 4, 4, 0},    
      /* 0x4e */ (Op){"LD", LD, C, a_HL, 0, 8, 8, 0}, 
      /* 0x4f */ (Op){"LD", LD, C, A, 0, 4, 4, 0},    
      /* 0x50 */ (Op){"LD", LD, D, B, 0, 4, 4, 0},    
      /* 0x51 */ (Op){"LD", LD, D, C, 0, 4, 4, 0},    
      /* 0x52 */ (Op){"LD", LD, D, D, 0, 4, 4, 0},    
      /* 0x53 */ (Op){"LD", LD, D, E, 0, 4, 4, 0},    
      /* 0x54 */ (Op){"LD", LD, D, H, 0, 4, 4, 0},    
      /* 0x55 */ (Op){"LD", LD, D, L, 0, 4, 4, 0},    
      /* 0x56 */ (Op){"LD", LD, D, a_HL, 0, 8, 8, 0}, 
      /* 0x57 */ (Op){"LD", LD, D, A, 0, 4, 4, 0},    
      /* 0x58 */ (Op){"LD", LD, E, B, 0, 4, 4, 0},    
      /* 0x59 */ (Op){"LD", LD, E, C, 0, 4, 4, 0},    
      /* 0x5a */ (Op){"LD", LD, E, D, 0, 4, 4, 0},    
      /* 0x5b */ (Op){"LD", LD, E, E, 0, 4, 4, 0},    
      /* 0x5c */ (Op){"LD", LD, E, H, 0, 4, 4, 0},    
      /* 0x5d */ (Op){"LD", LD, E, L, 0, 4, 4, 0},    
      /* 0x5e */ (Op){"LD", LD, E, a_HL, 0, 8, 8, 0}, 
      /* 0x5f */ (Op){"LD", LD, E, A, 0, 4, 4, 0},    
      /* 0x60 */ (Op){"LD", LD, H, B, 0, 4, 4, 0},    
      /* 0x61 */ (Op){"LD", LD, H, C, 0, 4, 4, 0},    
      /* 0x62 */ (Op){"LD", LD, H, D, 0, 4, 4, 0},    
      /* 0x63 */ (Op){"LD", LD, H, E, 0, 4, 4, 0},    
      /* 0x64 */ (Op){"LD", LD, H, H, 0, 4, 4, 0},    
      /* 0x65 */ (Op){"LD", LD, H, L, 0, 4, 4, 0},    
      /* 0x66 */ (Op){"LD", LD, H, a_HL, 0, 8, 8, 0}, 
      /* 0x67 */ (Op){"LD", LD, H, A, 0, 4, 4, 0},    
      /* 0x68 */ (Op){"LD", LD, L, B, 0, 4, 4, 0},    
      /* 0x69 */ (Op){"LD", LD, L, C, 0, 4, 4, 0},    
      /* 0x6a */ (Op){"LD", LD, L, D, 0, 4, 4, 0},    
      /* 0x6b */ (Op){"LD", LD, L, E, 0, 4, 4, 0},    
      /* 0x6c */ (Op){"LD", LD, L, H, 0, 4, 4, 0},    
      /* 0x6d */ (Op){"LD", LD, L, L, 0, 4, 4, 0},    
      /* 0x6e */ (Op){"LD", LD, L, a_HL, 0, 8, 8, 0}, 
      /* 0x6f */ (Op){"LD", LD, L, A, 0, 4, 4, 0},    
      /* 0x70 */ (Op){"LD", LD, a_HL, B, 0, 8, 8, 0}, 
      /* 0x71 */ (Op){"LD", LD, a_HL, C, 0, 8, 8, 0}, 
      /* 0x72 */ (Op){"LD", LD, a_HL, D, 0, 8, 8, 0}, 
      /* 0x73 */ (Op){"LD", LD, a_HL, E, 0, 8, 8, 0}, 
      /* 0x74 */ (Op){"LD", LD, a_HL, H, 0, 8, 8, 0}, 
      /* 0x75 */ (Op){"LD", LD, a_HL, L, 0, 8, 8, 0}, 
      /* 0x76 */ (Op){"HALT", HALT, NULL, NULL, 0, 4, 4, 0},
      /* 0x77 */ (Op){"LD", LD, a_HL, A, 0, 8, 8, 0},                
      /* 0x78 */ (Op){"LD", LD, A, B, 0, 4, 4, 0},                   
      /* 0x79 */ (Op){"LD", LD, A, C, 0, 4, 4, 0},                   
      /* 0x7a */ (Op){"LD", LD, A, D, 0, 4, 4, 0},                   
      /* 0x7b */ (Op){"LD", LD, A, E, 0, 4, 4, 0},                   
      /* 0x7c */ (Op){"LD", LD, A, H, 0, 4, 4, 0},                   
      /* 0x7d */ (Op){"LD", LD, A, L, 0, 4, 4, 0},                   
      /* 0x7e */ (Op){"LD", LD, A, a_HL, 0, 8, 8, 0},                
      /* 0x7f */ (Op){"LD", LD, A, A, 0, 4, 4, 0},                   
      /* 0x80 */ (Op){"ADD", ADD, A, B, 0, 1, 1, ZZ | N0 | HH | CC}, // << timings incomplete
      /* 0x81 */ (Op){"ADD", ADD, A, C, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x82 */ (Op){"ADD", ADD, A, D, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x83 */ (Op){"ADD", ADD, A, E, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x84 */ (Op){"ADD", ADD, A, H, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x85 */ (Op){"ADD", ADD, A, L, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x86 */ (Op){"ADD", ADD, A, a_HL, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x87 */ (Op){"ADD", ADD, A, A, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x88 */ (Op){"ADC", ADC, A, B, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x89 */ (Op){"ADC", ADC, A, C, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x8a */ (Op){"ADC", ADC, A, D, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x8b */ (Op){"ADC", ADC, A, E, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x8c */ (Op){"ADC", ADC, A, H, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x8d */ (Op){"ADC", ADC, A, L, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x8e */ (Op){"ADC", ADC, A, a_HL, 0, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0x8f */ (Op){"ADC", ADC, A, A, 0, 1, 1, ZZ | N0 | HH | CC},    
      /* 0x90 */ (Op){"SUB", SUB, B, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x91 */ (Op){"SUB", SUB, C, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x92 */ (Op){"SUB", SUB, D, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x93 */ (Op){"SUB", SUB, E, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x94 */ (Op){"SUB", SUB, H, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x95 */ (Op){"SUB", SUB, L, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x96 */ (Op){"SUB", SUB, a_HL, NULL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x97 */ (Op){"SUB", SUB, A, NULL, 0, 1, 1, ZZ | N1 | HH | CC},   
      /* 0x98 */ (Op){"SBC", SBC, A, B, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x99 */ (Op){"SBC", SBC, A, C, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x9a */ (Op){"SBC", SBC, A, D, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x9b */ (Op){"SBC", SBC, A, E, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x9c */ (Op){"SBC", SBC, A, H, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x9d */ (Op){"SBC", SBC, A, L, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0x9e */ (Op){"SBC", SBC, A, a_HL, 0, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0x9f */ (Op){"SBC", SBC, A, A, 0, 1, 1, ZZ | N1 | HH | CC},    
      /* 0xa0 */ (Op){"AND", AND, B, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa1 */ (Op){"AND", AND, C, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa2 */ (Op){"AND", AND, D, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa3 */ (Op){"AND", AND, E, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa4 */ (Op){"AND", AND, H, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa5 */ (Op){"AND", AND, L, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa6 */ (Op){"AND", AND, a_HL, NULL, 0, 1, 1, ZZ | N0 | H1 | C0}, 
      /* 0xa7 */ (Op){"AND", AND, A, NULL, 0, 1, 1, ZZ | N0 | H1 | C0},    
      /* 0xa8 */ (Op){"XOR", XOR, B, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xa9 */ (Op){"XOR", XOR, C, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xaa */ (Op){"XOR", XOR, D, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xab */ (Op){"XOR", XOR, E, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xac */ (Op){"XOR", XOR, H, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xad */ (Op){"XOR", XOR, L, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xae */ (Op){"XOR", XOR, a_HL, NULL, 0, 1, 1, ZZ | N0 | H0 | C0}, 
      /* 0xaf */ (Op){"XOR", XOR, A, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},    
      /* 0xb0 */ (Op){"OR", OR, B, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb1 */ (Op){"OR", OR, C, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb2 */ (Op){"OR", OR, D, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb3 */ (Op){"OR", OR, E, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb4 */ (Op){"OR", OR, H, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb5 */ (Op){"OR", OR, L, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb6 */ (Op){"OR", OR, a_HL, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},   
      /* 0xb7 */ (Op){"OR", OR, A, NULL, 0, 1, 1, ZZ | N0 | H0 | C0},      
      /* 0xb8 */ (Op){"CP", CP, B, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xb9 */ (Op){"CP", CP, C, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xba */ (Op){"CP", CP, D, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xbb */ (Op){"CP", CP, E, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xbc */ (Op){"CP", CP, H, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xbd */ (Op){"CP", CP, L, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xbe */ (Op){"CP", CP, a_HL, NULL, 0, 1, 1, ZZ | N1 | HH | CC},   
      /* 0xbf */ (Op){"CP", CP, A, NULL, 0, 1, 1, ZZ | N1 | HH | CC},      
      /* 0xc0 */ (Op){"RET", RET, f_NZ, NULL, 0, 1, 1, 0},                 
      /* 0xc1 */ (Op){"POP", POP, BC, NULL, 0, 1, 1, 0},                   
      /* 0xc2 */ (Op){"JP", JP, f_NZ, a16, 2, 1, 1, 0},               
      /* 0xc3 */ (Op){"JP", JP, a16, NULL, 2, 1, 1, 0},               
      /* 0xc4 */ (Op){"CALL", CALL, f_NZ, a16, 2, 1, 1, 0},           
      /* 0xc5 */ (Op){"PUSH", PUSH, BC, NULL, 0, 1, 1, 0},            
      /* 0xc6 */ (Op){"ADD", ADD, A, d8, 1, 1, 1, ZZ | N0 | HH | CC}, 
      /* 0xc7 */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xc8 */ (Op){"RET", RET, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xc9 */ (Op){"RET", RET, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xca */ (Op){"JP", JP, f_Z, a16, 2, 1, 1, 0},     
      /* 0xcb */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},  // PREFIX
      /* 0xcc */ (Op){"CALL", CALL, f_Z, a16, 2, 1, 1, 0}, 
      /* 0xcd */ (Op){"CALL", CALL, a16, NULL, 2, 1, 1, 0},
      /* 0xce */ (Op){"ADC", ADC, A, d8, 1, 1, 1, 0},      
      /* 0xcf */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xd0 */ (Op){"RET", RET, f_NC, NULL, 0, 1, 1, 0},
      /* 0xd1 */ (Op){"POP", POP, DE, NULL, 0, 1, 1, 0},  
      /* 0xd2 */ (Op){"JP", JP, f_NC, a16, 2, 1, 1, 0},   
      /* 0xd3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xd4 */ (Op){"CALL", CALL, f_NC, a16, 2, 1, 1, 0}, 
      /* 0xd5 */ (Op){"PUSH", PUSH, DE, NULL, 0, 1, 1, 0},  
      /* 0xd6 */ (Op){"SUB", SUB, d8, NULL, 1, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0xd7 */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xd8 */ (Op){"RET", RET, C, NULL, 0, 1, 1, 0}, 
      /* 0xd9 */ (Op){"RETI", RETI, NULL, NULL, 0, 1, 1, 0},
      /* 0xda */ (Op){"JP", JP, C, a16, 2, 1, 1, 0}, 
      /* 0xdb */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xdc */ (Op){"CALL", CALL, C, a16, 2, 1, 1, 0}, 
      /* 0xdd */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xde */ (Op){"SBC", SBC, A, d8, 1, 1, 1, ZZ | N1 | HH | CC}, 
      /* 0xdf */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xe0 */ (Op){"LD", LD, a_FF00_a8, A, 1, 1, 1, 0}, 
      /* 0xe1 */ (Op){"POP", POP, HL, NULL, 0, 1, 1, 0},   
      /* 0xe2 */ (Op){"LD", LD, a_FF00_C, A, 0, 1, 1, 0},  
      /* 0xe3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xe4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xe5 */ (Op){"PUSH", PUSH, HL, NULL, 0, 1, 1, 0},
      /* 0xe6 */ (Op){"AND", AND, d8, NULL, 1, 1, 1, ZZ | N0 | H1 | C0},
      /* 0xe7 */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xe8 */ (Op){"ADD", ADD, SP, r8, 1, 1, 1, Z0 | N0 | H1 | C0},
      /* 0xe9 */ (Op){"JP", JP, a_HL, NULL, 0, 1, 1, 0},
      /* 0xea */ (Op){"LD", LD, a_a16, A, 2, 1, 1, 0},
      /* 0xeb */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xec */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xed */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xee */ (Op){"XOR", XOR, d8, NULL, 1, 1, 1, ZZ | N0 | H0 | C0},
      /* 0xef */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xf0 */ (Op){"LD", LD, A, a_FF00_a8, 1, 1, 1, 0},
      /* 0xf1 */
      (Op){"POP", POP, NULL, /* TODO */ NULL, 0, 1, 1, ZZ | NN | HH | CC},
      /* 0xf2 */ (Op){"LD", LD, A, a_FF00_C, 1, 1, 1, 0},
      /* 0xf3 */ (Op){"DI", DI, NULL, NULL, 0, 1, 1, 0},
      /* 0xf4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xf5 */ (Op){"PUSH", PUSH, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xf6 */ (Op){"OR", OR, d8, NULL, 1, 1, 1, 0},
      /* 0xf7 */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /* 0xf8 */ (Op){"LD", LD, HL, SP_r8, 1, 1, 1, Z0 | N0 | HH | CC},
      /* 0xf9 */ (Op){"LD", LD, SP, HL, 0, 1, 1, 0},
      /* 0xfa */ (Op){"LD", LD, A, a_a16, 2, 1, 1, 0},
      /* 0xfb */ (Op){"EI", EI, NULL, NULL, 0, 1, 1, 0},
      /* 0xfc */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xfd */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xfe */ (Op){"CP", CP, d8, NULL, 1, 1, 1, ZZ | N1 | HH | CC},
      /* 0xff */ (Op){"RST", RST, NULL, /* << TODO */ NULL, 0, 1, 1, 0},
      /*
       * PREFIX 0xCB Ops
       * */
      /* 0xCB00 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB01 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB02 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB03 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB04 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB05 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB06 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB07 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB08 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB09 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB0F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB10 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB11 */ (Op){"RL", RL, C, NULL, 0, 8, 8, ZZ | N0 | H0 | CC},
      /* 0xCB12 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB13 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB14 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB15 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB16 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB17 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB18 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB19 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB1F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB20 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB21 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB22 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB23 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB24 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB25 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB26 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB27 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB28 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB29 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB2F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB30 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB31 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB32 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB33 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB34 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB35 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB36 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB37 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB38 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB39 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB3F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB40 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB41 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB42 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB43 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB44 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB45 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB46 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB47 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB48 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB49 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB4F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB50 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB51 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB52 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB53 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB54 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB55 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB56 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB57 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB58 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB59 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB5F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB60 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB61 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB62 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB63 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB64 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB65 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB66 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB67 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB68 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB69 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB6F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB70 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB71 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB72 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB73 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB74 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB75 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB76 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB77 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB78 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB79 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB7A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB7B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB7C */ (Op){"BIT", BIT, i7, H, 0, 8, 8, ZZ | N0 | H1},
      /* 0xCB7D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB7E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB7F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB80 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB81 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB82 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB83 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB84 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB85 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB86 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB87 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB88 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB89 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB8F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB90 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB91 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB92 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB93 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB94 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB95 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB96 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB97 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB98 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB99 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9A */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9B */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9C */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9D */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9E */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCB9F */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBA9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAD */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBAF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBB9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBD */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBBF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBC9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCD */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBCF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBD9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDD */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBDF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBE9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBEA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBEB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBEC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBED */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBEE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBEF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF0 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF1 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF2 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF3 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF4 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF5 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF6 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF7 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF8 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBF9 */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFA */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFB */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFC */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFD */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFE */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
      /* 0xCBFF */ (Op){"NOP", NOP, NULL, NULL, 0, 1, 1, 0},
  }[0];
}

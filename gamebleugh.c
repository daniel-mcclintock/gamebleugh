#include "decode.h"
#include "gpu.h"
#include "mmu.h"
#include <string.h>
#include <unistd.h>

Op *get_op(CPU *cpu, Op *opmap, int *debug);
void dump_op(Op *op);
void dump_state(int8_t pc_offset, u_int8_t *op_byte, Op *op);
void loop(int *render, int *debug, int *tickstep);
void timer(u_int8_t *cycles);

int main(int argc, char *argv[]) {
  int tickstep = 0;
  int debug = 0;
  int render = 1;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--tickstep") == 0) {
      tickstep = 1;
    } else if (strcmp(argv[i], "--debug") == 0) {
      debug = 1;
    } else if (strcmp(argv[i], "--no-render") == 0) {
      render = 0;
    }
  }

  mmu_init("dmg.rom", "tetris.japan.en.gb");
  gpu_init();

  printf(TERM_HIDE_CURSOR TERM_HEAD TERM_CLEAR);
  loop(&render, &debug, &tickstep);
  printf(TERM_SHOW_CURSOR);

  return 0;
}

void loop(int *render, int *debug, int *tickstep) {
  /* main emulation tick loop */
  Op *op;
  Op *opmap = get_opmap();

  u_int16_t last_pc = 0;
  u_int8_t *byte;
  u_int8_t *ticks;

  do {
    op = get_op(&cpu, opmap, debug);
    ticks = op->op(&op->cycles_t, &op->cycles_f, op->arg1, op->arg2);
    apply_flags(op);

    timer(ticks);
    // TODO: handle_interrupts
    gpu_tick(ticks, render);
    cpu.registers.PC += op->length;

    if (*tickstep) {
      if (cpu.registers.PC - op->length > last_pc) {
        last_pc = cpu.registers.PC;
        getchar();
      }
    }

  } while (1);
}

void timer(u_int8_t *cycles) {
  static u_int16_t total_cycles = 0;
  total_cycles += *cycles;

  // TODO: Actual timer
  if (total_cycles > 16000) {
    usleep(5000); // magic!
    total_cycles = 0;
  }
}

Op *get_op(CPU *cpu, Op *opmap, int *debug) {
  /* get the current PC Operation, handles the prefix byte Op table */
  u_int8_t *byte = mmu_fetch(&cpu->registers.PC);

  cpu->registers.PC++;

  Op *op;
  int pc_offset = -1;

  if (*byte == PREFIX_BYTE) {
    byte = mmu_fetch(&cpu->registers.PC);
    cpu->registers.PC++;
    op = &opmap[*byte + PREFIX_BYTE_OFFSET];
    pc_offset = -2;
  } else {
    op = &opmap[*byte];
  }

  if (*debug) {
    dump_state(pc_offset, byte, op);
  }

  return op;
}

void dump_op(Op *op) {
  /* dump out some debug info about the given Op */
  printf("%-4s ", op->name);
  printf("%-11s ", op->arg1 != NULL ? op->arg1->name : "           ");
  printf("%-11s ", op->arg2 != NULL ? op->arg2->name : "           ");
}

void dump_state(int8_t pc_offset, u_int8_t *op_byte, Op *op) {
  /* dump out the current emulation state */
  printf("0x%02X ", *op_byte);
  dump_op(op);

  printf("PC=%04X ", cpu.registers.PC + pc_offset);
  printf("SP=%04X ", cpu.registers.SP);

  printf("FLAGS=%i", cpu.registers.FLAGS & 0x80 ? 1 : 0);
  printf("%i", cpu.registers.FLAGS & 0x40 ? 1 : 0);
  printf("%i", cpu.registers.FLAGS & 0x20 ? 1 : 0);
  printf("%i ", cpu.registers.FLAGS & 0x41 ? 1 : 0);

  printf("A=%02X ", cpu.registers.A);
  printf("B=%02X ", cpu.registers.B);
  printf("C=%02X ", cpu.registers.C);
  printf("D=%02X ", cpu.registers.D);
  printf("E=%02X ", cpu.registers.E);
  printf("H=%02X ", cpu.registers.H);
  printf("L=%02X ", cpu.registers.L);
  printf("X=%i ", *mmu_fetch(&ADDR_VIEW_X));
  printf("Y=%i\n", *mmu_fetch(&ADDR_VIEW_Y));
}

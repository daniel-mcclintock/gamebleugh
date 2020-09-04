#include <stdio.h>
#include <stdlib.h>

#include "gpu.h"
#include "mmu.h"

u_int8_t in_boot_rom = 1;
u_int8_t boot_rom[0x00FF];
u_int8_t rom[0x8000];
u_int8_t eram[0x2000];
u_int8_t wram[0x2000];
u_int8_t zram[0x007F];
u_int8_t mmio[0x00FF];

void mmu_init(const char *bootrompath, const char *rompath) {
  /* read in boot.rom */
  FILE *fp = fopen(bootrompath, "r");
  fread(&boot_rom, sizeof(u_int8_t), 255, fp);
  fclose(fp);

  /* read in cart rom */
  fp = fopen(rompath, "r");
  fread(&rom, sizeof(u_int8_t), 0x8000, fp);
  fclose(fp);

  /*
    Magic cart addresses

    0x0134 -> 0x0143: Title of the game in ASCII zero terminated
    0x0144 -> 0x0145:

    cart_publisher = [0x0144:0x0145]
    cart_super_game_boy = [0x0146]
    self.cart_type = [0x0147]
    cart_rom_size = [0x0148]
    cart_ram_size = [0x0149]
    cart_market_destination = [0x014A]
    cart_publisher = [0x014B]
    cart_version = [0x014C]
    cart_header_checksum = [0x014D]
    cart_rom_checksum = [0x014E:0x014F]

    CART Type

    0x00: No MBC
    0x01: MBC1
    0x02: MBC1 with external RAM
    0x03: MBC1 with battery-backed external RAM
  */
}

u_int8_t *mmu_fetch(u_int16_t *addr) {
  static u_int8_t zero = 0;

  switch (*addr & 0xF000) {
  case 0x0000:
    // Boot ROM / Cart ROM 0
    if (in_boot_rom) {
      if (*addr < 0x0100) {
        return &boot_rom[*addr];
      }

      if (*addr == 0x0100) {
        in_boot_rom = 0;
      }

      return &rom[*addr];
    }
  case 0x1000:
  case 0x2000:
  case 0x3000:
    // Cart ROM0
    return &rom[*addr];
  case 0x4000:
  case 0x5000:
  case 0x6000:
  case 0x7000:
    // Cart ROM1, No MBC
    return &rom[*addr];
  case 0x8000:
  case 0x9000:
    // GPU VRAM
    return gpu_fetch(addr);
  case 0xA000:
  case 0xB000:
    // Ext RAM
    return &eram[*addr & 0x1FFF];
  case 0xC000:
  case 0xD000:
    // Working RAM
    return &wram[*addr & 0x1FFF];
  case 0xE000:
    // Working RAM Shadow
    return &wram[*addr & 0x1FFF];
  case 0xF000:
    // Working RAM Shadow, I/O, ZRAM
    switch (*addr & 0x0F00) {
    case 0x0000:
    case 0x0100:
    case 0x0200:
    case 0x0300:
    case 0x0400:
    case 0x0500:
    case 0x0600:
    case 0x0700:
    case 0x0800:
    case 0x0900:
    case 0x0A00:
    case 0x0B00:
    case 0x0C00:
    case 0x0D00:
      // Working RAM Shadow
      // TODO: Double check this
      return &wram[*addr & 0x1FFF];

    case 0x0E00:
      // GFX OAM, 160-bytes, the rest 0
      if (*addr < 0xFEA0) {
        return gpu_fetch(addr);
      }

      return &zero;
    }
  case 0x0F00:
    // ZRAM, I/O
    if (*addr >= 0xFF80) {
      // TODO: This is wrong
      return &zram[*addr & 0x007F];
    } else if (*addr >= 0xFF00) {
      // MMIO
      return &zram[*addr & 0x00FF];
    }
  }

  printf("Unhandled memory fetch");
  return &zero;
}
void mmu_write(u_int16_t *addr, u_int8_t *value) {
  // TODO: So part of this i am just trusting that a write is possible.
  // They probably should be read only
  switch (*addr & 0xF000) {
  case 0x0000:
    // Boot ROM / Cart ROM 0
    if (in_boot_rom) {
      if (*addr < 0x0100) {
        boot_rom[*addr] = *value;
      }

      if (*addr == 0x0100) {
        in_boot_rom = 0;
      }

      rom[*addr] = *value;
    }
    return;
  case 0x1000:
  case 0x2000:
  case 0x3000:
    // Cart ROM0
    rom[*addr] = *value;
    return;
  case 0x4000:
  case 0x5000:
  case 0x6000:
  case 0x7000:
    // Cart ROM1, No MBC
    rom[*addr] = *value;
    return;
  case 0x8000:
  case 0x9000:
    // GPU VRAM
    gpu_write(addr, value);
    return;
  case 0xA000:
  case 0xB000:
    // Ext RAM
    eram[*addr & 0x1FFF] = *value;
    return;
  case 0xC000:
  case 0xD000:
    // Working RAM
    wram[*addr & 0x1FFF] = *value;
    return;
  case 0xE000:
    // Working RAM Shadow
    wram[*addr & 0x1FFF] = *value;
    return;
  case 0xF000:
    // Working RAM Shadow, I/O, ZRAM
    switch (*addr & 0x0F00) {
    case 0x0000:
    case 0x0100:
    case 0x0200:
    case 0x0300:
    case 0x0400:
    case 0x0500:
    case 0x0600:
    case 0x0700:
    case 0x0800:
    case 0x0900:
    case 0x0A00:
    case 0x0B00:
    case 0x0C00:
    case 0x0D00:
      // Working RAM Shadow
      // TODO: Double check this
      wram[*addr & 0x1FFF] = *value;
      return;
    case 0x0E00:
      // GFX OAM, 160-bytes, the rest 0
      if (*addr < 0xFEA0) {
        // gpu fetch
        gpu_write(addr, value);
      }

      return;
    }
  case 0x0F00:
    // ZRAM, I/O
    if (*addr >= 0xFF80) {
      // TODO: Is this wrong?
      zram[*addr & 0x007F] = *value;
      return;
    } else if (*addr >= 0xFF00) {
      // MMIO
      zram[*addr & 0x00FF] = *value;
      return;
    }
    return;
  }

  printf("Unhandled memory fetch");
}

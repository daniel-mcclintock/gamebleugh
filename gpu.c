#include "gpu.h"
#include "mmu.h"
#include <sixel.h>
#include <stdio.h>
#include <unistd.h>

/*
   Sequentual space for VRAM -> 0x0000 -> 0x209F
   -> 0x8000 -> 0x87FF Tiles 0 0-127
   -> 0x8800 -> 0x8FFF Tiles 0 and 1 128-255
   -> 0x9000 -> 0x97FF Tiles 1 0-127
   -> 0x9800 -> 0x9BFF Tile map 0
   -> 0x9C00 -> 0x9FFF Tile map 1
   -> 0xFE00 -> 0xFE9F Sprite data(positions etc)

NOTE: The crossover of Tiles 0 and 1, resulting in a total of 384 unique tiles.
*/
u_int8_t vram[0x209F];

// RGB-ish pallete
const u_int8_t PALLETE[4] = {0xFF, 0xC0, 0x60, 0x00};

// tile_id, tile_y, tile_x
const u_int8_t *tile_pixel_cache[384][8][8];

u_int8_t buffer[160 * 144 * 8];
sixel_dither_t *dither;
sixel_output_t *output = NULL;

static int sixel_write(char *data, int size, void *priv) {
  return fwrite(data, 1, size, (FILE *)priv);
};

u_int16_t addr_to_gpu_addr(u_int16_t *addr) {
  // TODO: Is this right?
  if (*addr > 0x9FFF) {
    return 0x1FFF + (*addr & 0x00FF);
  }

  return *addr & 0x1FFF;
}

void gpu_init() {
  dither = sixel_dither_get(SIXEL_BUILTIN_G8);
  sixel_dither_set_pixelformat(dither, SIXEL_PIXELFORMAT_G8);
  if (SIXEL_FAILED(sixel_output_new(&output, sixel_write, stdout, NULL))) {
    printf("failed to create sixel output");
  }

  // Preload white into all pixels in tile_pixel_cache
  for (int id = 0; id < 384; id++) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        tile_pixel_cache[id][y][x] = &PALLETE[0];
      }
    }
  }
}

u_int8_t *gpu_fetch(u_int16_t *addr) {
  /*
    Handle GPU VRAM & OAM reads.

    TODO: Use the MMU to translate addresses rather than handle it here.

    The MMU will redirect reads here but performs no translation of the
    addresses, as such they need to be re-mapped for the GPU through
    `addr_to_gpu_addr`
  */

  return &vram[addr_to_gpu_addr(addr)];
}

void gpu_write(u_int16_t *addr, u_int8_t *value) {
  /*
    Handle GPU VRAM & OAM writes.

    TODO: Use the MMU to translate addresses rather than handle it here.

    The MMU will redirect writes here but performs no translation of the
    addresses, as such they need to be re-mapped for the GPU through
    `addr_to_gpu_addr`

    Also performs extra processing on tile data writes to cache tile pixel 
    colors for easy lookups during rendering.
  */

  vram[addr_to_gpu_addr(addr)] = *value;

  /*
     Internal Tile addresses:
     0x0000 -> 0x0FFF Tiles 0
     0x0800 -> 0x17FF Tiles 1
     0x1800 -> 0x1BFF Tile map 0
     0x1C00 -> 0x1FFF Tile map 1

     0x1F00 Tile addr mask

     Each tile is 8x8 pixels, each pixel's colour is determined by 2 bits,
     offset 1-byte from the initial bit.

     Bits to colour mapping:

     Bits  | Value | Description | RGB-ish
     ------|-------|-------------|----------
     [0,0] | 0     | Off         | (255, 255, 255)
     [1,0] | 1     | 33% on      | (192, 192, 192)
     [0,1] | 2     | 66% on      | (96, 96, 96)
     [1,1] | 3     | On          | (0, 0, 0)

     In terms of data size this means each tile is stored with 8x16 bits.

     Eg;
           // Tile data
            x x x x x x x x
           |0|1|2|3|4|5|6|7
           |--------------- // Example colour decode
        y00|0|1|0|0|1|1|1|0    2,  1,  0,   0,   3,  1,  3,  2
        y01|1|0|0|0|1|0|1|1    66, 33, off, off, on, 33, on, 66
           |---------------
        y02|0|0|0|0|0|0|0|0
        y03|0|0|0|0|0|0|0|0
           |---through-15--
        y14|0|0|0|0|0|0|0|0
        y15|0|0|0|0|0|0|0|0
  */

  // Decode and translate Tile writes to Tilecache
  if ((*addr & 0x1F00) <= 0x17FF) {
    // Tile 0-1 data
    u_int16_t tile_row_addr = *addr & 0x17FE;
    u_int16_t tile_id = (tile_row_addr >> 4) & 511;
    u_int8_t tile_y = (*addr & 15) >> 1;
    u_int8_t tile_row_0 = vram[tile_row_addr];
    u_int8_t tile_row_1 = vram[tile_row_addr + 1];

    // Determine the color of this pixel row and store in tile_pixel_cache
    for (int tile_x = 0; tile_x < 8; tile_x++) {

      u_int8_t x_mask = 1 << (7 - tile_x);
      const u_int8_t *colour = &PALLETE[(tile_row_0 & x_mask ? 1 : 0) +
                                        (tile_row_1 & x_mask ? 2 : 0)];

      tile_pixel_cache[tile_id][tile_y][tile_x] = colour;
    }
  }
}

void gpu_render(u_int8_t *view_x, u_int8_t *view_y) {
  /*
    0x1800 -> 0x1BFF Tile map 0
    0x1C00 -> 0x1FFF Tile map 1
  */

  printf(TERM_HEAD);

  int buffer_i = 0;
  for (u_int8_t scan_y = *view_y; scan_y <= 144 + *view_y; scan_y++) {
    // offset memory addr used for fetching the tile ids
    u_int16_t map_y_stride = (((*view_y) + scan_y) >> 3) * 32;
    u_int8_t tile_y = ((*view_y) + scan_y) & 7;

    for (u_int8_t scan_x = *view_x; scan_x < 160 + *view_x; scan_x++) {
      u_int16_t map_x = scan_x >> 3;
      u_int16_t map_addr = 0x1800 + map_x + map_y_stride;
      u_int8_t tile_x = scan_x & 7;
      u_int8_t tile_id = vram[map_addr];

      u_int8_t pixel = *tile_pixel_cache[tile_id][tile_y][tile_x];
      buffer[buffer_i++] = pixel;
    }
  }

  sixel_encode(&buffer[0], 160, 144, 0, dither, output);
}

void gpu_tick(u_int8_t *ticks, int *render) {
  static int mode = MODE_OAM;
  static int mode_ticks = 0;
  static u_int8_t scan_y = 0;

  mode_ticks += *ticks;

  if (mode == MODE_OAM) {
    if (mode_ticks >= 80) {
      mode_ticks = 0;
      mode = MODE_LINERENDER;
    }
  } else if (mode == MODE_LINERENDER) {
    if (mode_ticks >= 172) {
      mode_ticks = 0;
      mode = MODE_HBLANK;

      scan_y++;
      mmu_write(&ADDR_SCAN_Y, &scan_y);
    }
  } else if (mode == MODE_HBLANK) {
    if (mode_ticks >= 204) {
      mode_ticks = 0;

      if (scan_y == 144) {
        mode = MODE_VBLANK;
      } else {
        mode = MODE_OAM;
      }
    }
  } else if (mode == MODE_VBLANK) {
    if (mode_ticks >= 456) {
      mode_ticks = 0;
      scan_y++;

      if (scan_y == 153) {
        mode_ticks = 0;
        if (*render) {
          gpu_render(mmu_fetch(&ADDR_VIEW_X), mmu_fetch(&ADDR_VIEW_Y));
        }
        scan_y = 0;
        mode = MODE_OAM;
      }
      mmu_write(&ADDR_SCAN_Y, &scan_y);
    }
  }
}

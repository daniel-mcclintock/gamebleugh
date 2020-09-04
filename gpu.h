#ifndef GPU_H
#define GPU_H
#include <stdlib.h>

#define MODE_OAM 0
#define MODE_LINERENDER 1
#define MODE_HBLANK 2
#define MODE_VBLANK 3

#define TERM_HEAD "\033[H"
#define TERM_CLEAR "\033[2J"
#define TERM_HIDE_CURSOR "\033[?25l"
#define TERM_SHOW_CURSOR "\033[?25h"

static u_int16_t ADDR_SCAN_Y = 0xFF44;
static u_int16_t ADDR_VIEW_X = 0xFF43;
static u_int16_t ADDR_VIEW_Y = 0xFF42;

void gpu_init();
u_int8_t *gpu_fetch(u_int16_t *addr);
void gpu_write(u_int16_t *addr, u_int8_t *value);
void gpu_tick(u_int8_t *ticks, int *render);

#endif

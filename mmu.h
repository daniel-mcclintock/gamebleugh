#ifndef MMU_H
#define MMU_H
#include <stdlib.h>

void mmu_init(const char *bootrompath, const char *rompath);
u_int8_t *mmu_fetch(u_int16_t *addr);
void mmu_write(u_int16_t *addr, u_int8_t *value);

#endif

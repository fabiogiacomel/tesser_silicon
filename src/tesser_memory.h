#ifndef TESSER_MEMORY_H
#define TESSER_MEMORY_H

#include <stdint.h>

// Definições do Pool
#define POOL_START_ADDR 0x2000
#define POOL_BLOCK_SIZE 64
#define POOL_BLOCK_COUNT 64 // Gerencia 4KB de RAM (64 * 64 = 4096 bytes)

void memory_pool_init();
uint16_t pool_alloc();
void pool_free(uint16_t addr);

#endif // TESSER_MEMORY_H

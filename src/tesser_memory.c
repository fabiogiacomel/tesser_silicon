#include "tesser_memory.h"
#include <stdio.h>
#include <string.h>

// Bitmap para rastrear alocações (64 blocos = 64 bits = 8 bytes)
// uint64_t seria ideal, mas para portabilidade 8 bits em array
static uint8_t pool_bitmap[POOL_BLOCK_COUNT / 8];

void memory_pool_init() {
    memset(pool_bitmap, 0, sizeof(pool_bitmap));
    printf("[MEM] Memory Pool Initialized. Region: 0x%04X - 0x%04X\n", 
           POOL_START_ADDR, POOL_START_ADDR + (POOL_BLOCK_COUNT * POOL_BLOCK_SIZE));
}

uint16_t pool_alloc() {
    for (int byte_idx = 0; byte_idx < (POOL_BLOCK_COUNT / 8); byte_idx++) {
        if (pool_bitmap[byte_idx] != 0xFF) { // Se tem espaço neste byte
            for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
                if (!(pool_bitmap[byte_idx] & (1 << bit_idx))) {
                    // Encontrei bloco livre
                    int block_id = (byte_idx * 8) + bit_idx;
                    
                    // Marcar como ocupado
                    pool_bitmap[byte_idx] |= (1 << bit_idx);
                    
                    // Calcular endereço físico
                    uint16_t addr = POOL_START_ADDR + (block_id * POOL_BLOCK_SIZE);
                    // printf("[MEM] Alloc Block %d @ 0x%04X\n", block_id, addr);
                    return addr;
                }
            }
        }
    }
    printf("[MEM] Alloc Failed: Out of Memory!\n");
    return 0; // Null/Error
}

void pool_free(uint16_t addr) {
    if (addr < POOL_START_ADDR) return;
    
    int offset = addr - POOL_START_ADDR;
    int block_id = offset / POOL_BLOCK_SIZE;
    
    if (block_id >= POOL_BLOCK_COUNT) return;
    
    int byte_idx = block_id / 8;
    int bit_idx = block_id % 8;
    
    // Limpar bit
    pool_bitmap[byte_idx] &= ~(1 << bit_idx);
    // printf("[MEM] Free Block %d @ 0x%04X\n", block_id, addr);
}

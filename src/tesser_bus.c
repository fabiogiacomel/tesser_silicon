#include "tesser_bus.h"
#include <stdio.h>

// Memória de 64KB (RAM Simples)
static uint8_t memory[65536];

uint8_t bus_read(uint16_t addr) {
    return memory[addr];
}

void bus_write(uint16_t addr, uint8_t val) {
    memory[addr] = val;
}

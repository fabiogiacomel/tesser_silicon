#ifndef TESSER_BUS_H
#define TESSER_BUS_H

#include <stdint.h>

// Declaração das funções de acesso ao barramento (memória)
void bus_write(uint16_t addr, uint8_t val);
uint8_t bus_read(uint16_t addr);

#endif // TESSER_BUS_H

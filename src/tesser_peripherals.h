#ifndef TESSER_PERIPHERALS_H
#define TESSER_PERIPHERALS_H

#include <stdint.h>
#include <stdbool.h>

// Endereços de MMIO
#define MMIO_BASE       0xF000
#define AES_CONTROL     0xF000
#define AES_DATA_IN     0xF001
#define AES_DATA_OUT    0xF002

// Bits de Controle
#define AES_CTRL_START  0x01 // Bit 0
#define AES_CTRL_READY  0x02 // Bit 1

// Funções de interface
void peripherals_init();
void peripherals_tick(); // Avança o relógio dos periféricos
uint8_t mmio_read(uint16_t addr);
void mmio_write(uint16_t addr, uint8_t val);

#endif // TESSER_PERIPHERALS_H

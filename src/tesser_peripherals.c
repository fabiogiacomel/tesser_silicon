#include "tesser_peripherals.h"
#include <stdio.h>

// Estrutura interna para o estado do AES
typedef struct {
    uint8_t control;
    uint8_t data_in;
    uint8_t data_out;
    bool is_busy;
    int cycles_remaining;
} AESState;

static AESState aes;

void peripherals_init() {
    aes.control = 0;
    aes.data_in = 0;
    aes.data_out = 0;
    aes.is_busy = false;
    aes.cycles_remaining = 0;
}

void peripherals_tick() {
    if (aes.is_busy) {
        if (aes.cycles_remaining > 0) {
            aes.cycles_remaining--;
        }
        
        if (aes.cycles_remaining == 0) {
            // Operação concluída
            aes.data_out = aes.data_in ^ 0xFF; // Operação XOR simples como placeholder
            aes.control |= AES_CTRL_READY;     // Levanta bit READY
            aes.control &= ~AES_CTRL_START;    // Limpa bit START (opcional, comum em hw)
            aes.is_busy = false;
            // printf("[HW] AES Operation Complete. Result: 0x%02X\n", aes.data_out);
        }
    }
}

uint8_t mmio_read(uint16_t addr) {
    switch (addr) {
        case AES_CONTROL:
            return aes.control;
        case AES_DATA_IN:
            return aes.data_in;
        case AES_DATA_OUT:
            return aes.data_out;
        default:
            printf("[MMIO] Read from unimplemented address 0x%04X\n", addr);
            return 0;
    }
}

void mmio_write(uint16_t addr, uint8_t val) {
    switch (addr) {
        case AES_CONTROL:
            // Se escrever bit START e não estiver ocupado
            if ((val & AES_CTRL_START) && !aes.is_busy) {
                aes.is_busy = true;
                aes.cycles_remaining = 5; // Latência simulada
                aes.control = val & ~AES_CTRL_READY; // Limpa ready
                // printf("[HW] AES Started. Cycles: %d\n", aes.cycles_remaining);
            }
            // Não permitimos escrever no bit READY diretamente
            break;
            
        case AES_DATA_IN:
            aes.data_in = val;
            break;
            
        case AES_DATA_OUT:
            // Read-only logic usually, implies ignored write or specific behavior
            // For now, allow overwrite or ignore? Ignoring is safer for 'OUT' register emulation
            // aes.data_out = val; 
            break;
            
        default:
            printf("[MMIO] Write to unimplemented address 0x%04X val 0x%02X\n", addr, val);
            break;
    }
}

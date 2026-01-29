#include "tesser_bus.h"
#include "tesser_cpu.h"
#include "tesser_peripherals.h"
#include <stdio.h>

// Definições de Memória
#define KERNEL_BASE 0x0000
#define KERNEL_LIMIT 0x0FFF
// RAM simulada
uint8_t memory[65536] = {0};

static int check_mpu(uint16_t addr, int is_write) {
    if (g_cpu_context == NULL) return 1; // Boot/Init safe mode

    // Se estiver em modo USER
    if (g_cpu_context->current_privilege_level == PRIV_USER) {
        // Proteção da Área de Kernel (0x0000 - 0x0FFF)
        if (addr <= KERNEL_LIMIT) {
            printf("\n[MPU ALERT] Access Violation! User Mode tried to %s Address 0x%04X\n", 
                is_write ? "WRITE" : "READ", addr);
            g_cpu_context->mpu_fault_triggered = 1;
            return 0; // Deny
        }
    }
    return 1; // Allow
}

uint8_t bus_read(uint16_t addr) {
    if (!check_mpu(addr, 0)) return 0xFF; // Return garbage on fault

    if (addr >= MMIO_BASE) {
        return mmio_read(addr);
    }
    return memory[addr];
}

void bus_write(uint16_t addr, uint8_t val) {
    if (!check_mpu(addr, 1)) return; // Access denied

    if (addr >= MMIO_BASE) {
        mmio_write(addr, val);
        return;
    }
    memory[addr] = val;
}

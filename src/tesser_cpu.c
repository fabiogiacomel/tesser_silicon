#include "tesser_cpu.h"
#include "tesser_bus.h"
#include <stdio.h>

// Definição do ponteiro global
TesserCPU *g_cpu_context = NULL;

/*
 * Instruction Set Architecture (Draft for Emulator):
 * ... (Previous ops)
 * 4. SETPRIV 0x05 [LEVEL] : Set Privilege Level (0=Kernel, 1=User) - Only valid in Kernel Mode?
 *    For simplicity in this test, we might just manipulate the struct directly in main, 
 *    or add a privileged instruction. I will rely on main manipulation as requested by "Integração".
 */

void cpu_step(TesserCPU *cpu) {
    // Update global context for Bus visibility
    g_cpu_context = cpu;

    if (cpu->mpu_fault_triggered) {
        printf("[CPU] Halted due to previous MPU Fault.\n");
        return;
    }

    // 1. Fetch
    // Note: Even fetching instructions should be subject to MPU, but usually Code is in ROM/Kernel space.
    // If User PC wanders into Kernel space, it should fault.
    // bus_read checks permissions.
    uint8_t opcode = bus_read(cpu->pc);
    
    // Check if fetch caused fault
    if (cpu->mpu_fault_triggered) return;

    cpu->pc++;

    switch (opcode) {
        case 0x01: // MOV reg, imm16
        {
            uint8_t reg_idx = bus_read(cpu->pc++);
            uint8_t val_hi = bus_read(cpu->pc++);
            uint8_t val_lo = bus_read(cpu->pc++);
            if (cpu->mpu_fault_triggered) return;

            if (reg_idx < 16) {
                cpu->regs[reg_idx] = (uint16_t)((val_hi << 8) | val_lo);
            }
            break;
        }
        case 0x02: // ADD dest, src
        {
            uint8_t dest_idx = bus_read(cpu->pc++);
            uint8_t src_idx = bus_read(cpu->pc++);
            if (cpu->mpu_fault_triggered) return;

            if (dest_idx < 16 && src_idx < 16) {
                cpu->regs[dest_idx] += cpu->regs[src_idx];
            }
            break;
        }
        case 0x03: // STORE addr, reg
        {
            uint8_t addr_hi = bus_read(cpu->pc++);
            uint8_t addr_lo = bus_read(cpu->pc++);
            uint8_t reg_idx = bus_read(cpu->pc++);
            if (cpu->mpu_fault_triggered) return;

            uint16_t addr = (uint16_t)((addr_hi << 8) | addr_lo);
            if (reg_idx < 16) {
                uint16_t val = cpu->regs[reg_idx];
                bus_write(addr, (val >> 8) & 0xFF);
                if (cpu->mpu_fault_triggered) return; // Check after first byte write
                bus_write(addr + 1, val & 0xFF);
            }
            break;
        }
        // ... (Include LOAD 0x04 if we kept it from previous thought, but user logic didn't explicitly demand it yet here)
        default:
            // printf("CPU: Unknown Opcode 0x%02X\n", opcode);
            break;
    }
}

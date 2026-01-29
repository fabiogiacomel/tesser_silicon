#include "tesser_cpu.h"
#include "tesser_bus.h"
#include <stdio.h>

// Definição do contexto global (para satisfazer linker do tesser_telemetry.c)
TesserCPU *g_cpu_context = NULL;

void cpu_step(TesserCPU *cpu) {
    // Atualiza contexto
    g_cpu_context = cpu;

    // 1. Fetch
    uint8_t opcode = bus_read(cpu->pc);
    cpu->pc++;

    // 2. Decode & Execute
    switch (opcode) {
        case OP_MOV: // MOV Dest, Imm16 (1 byte reg, 2 bytes val)
        {
            uint8_t reg_idx = bus_read(cpu->pc++);
            uint8_t val_hi = bus_read(cpu->pc++);
            uint8_t val_lo = bus_read(cpu->pc++);
            
            if (reg_idx < 16) {
                cpu->regs[reg_idx] = (uint16_t)((val_hi << 8) | val_lo);
            }
            break;
        }
        case OP_ADD: // ADD Dest, Src (1 byte dst, 1 byte src)
        {
            uint8_t dest_idx = bus_read(cpu->pc++);
            uint8_t src_idx = bus_read(cpu->pc++);
            
            if (dest_idx < 16 && src_idx < 16) {
                cpu->regs[dest_idx] += cpu->regs[src_idx];
            }
            break;
        }
        case OP_STORE: // STORE Src, Addr16 (1 byte src, 2 bytes addr)
        {
            uint8_t src_idx = bus_read(cpu->pc++);
            uint8_t addr_hi = bus_read(cpu->pc++);
            uint8_t addr_lo = bus_read(cpu->pc++);
            
            uint16_t addr = (uint16_t)((addr_hi << 8) | addr_lo);
            
            if (src_idx < 16) {
                uint16_t val = cpu->regs[src_idx];
                bus_write(addr, (val >> 8) & 0xFF);
                bus_write(addr + 1, val & 0xFF);
            }
            break;
        }
        default:
            break;
    }
}

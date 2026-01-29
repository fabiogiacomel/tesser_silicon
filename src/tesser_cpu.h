#ifndef TESSER_CPU_H
#define TESSER_CPU_H

#include <stdint.h>

// Definições de Opcodes
typedef enum {
    OP_MOV   = 0x01,
    OP_ADD   = 0x02,
    OP_STORE = 0x03
} Opcode;

// Estrutura da CPU
typedef struct {
    uint16_t regs[16]; // R0-R15
    uint16_t pc;       // Program Counter
    uint8_t flags;     // Status Flags
} TesserCPU;

// Contexto Global para Telemetria/MPU (Crucial para o Linker)
extern TesserCPU *g_cpu_context;

void cpu_step(TesserCPU *cpu);

#endif // TESSER_CPU_H

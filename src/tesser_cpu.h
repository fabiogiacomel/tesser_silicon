#ifndef TESSER_CPU_H
#define TESSER_CPU_H

#include <stdint.h>

typedef enum {
    PRIV_KERNEL = 0,
    PRIV_USER = 1
} PrivilegeLevel;

// Estrutura da CPU atualizada
typedef struct {
    uint16_t regs[16];
    uint16_t pc;
    uint16_t flags;
    PrivilegeLevel current_privilege_level;
    uint8_t mpu_fault_triggered; // Flag to indicate a fault occurred
} TesserCPU;

// Global pointer to allow Bus/MPU to check state (simple emulator pattern)
extern TesserCPU *g_cpu_context;

void cpu_step(TesserCPU *cpu);

#endif // TESSER_CPU_H

#ifndef TESSER_CPU_H
#define TESSER_CPU_H

#include <stdint.h>

// Opcodes da Arquitetura Stack Machine v1.0
typedef enum {
    OP_PUSH     = 0x01, // PUSH imm16
    OP_MUI_SET  = 0x02, // MUI_SET id8 (Pop val)
    OP_WAIT     = 0x03, // WAIT (Pop ms)
    OP_JMP      = 0x04, // JMP addr16
    OP_SMOOTH   = 0x05, // SMOOTH id8 (Pop val)
    OP_MUI_GET  = 0x06, // MUI_GET id8 (Push val)
    OP_SUB      = 0x07, // SUB (Pop A, Pop B -> Push A-B)
    OP_JMP_POS  = 0x08  // JMP_POS addr16 (Pop A, if A>0 jump)
} Opcode;

typedef struct {
    uint16_t stack[16]; // Pilha de Hardware de 16 posições
    uint8_t sp;         // Stack Pointer (0-15)
    uint16_t pc;        // Program Counter
} TesserCPU;

// Contexto Global (Mantido para compatibilidade com Telemetria)
extern TesserCPU *g_cpu_context;

void cpu_step(TesserCPU *cpu);

#endif // TESSER_CPU_H

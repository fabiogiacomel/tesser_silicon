#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h"
#include <stdio.h>

// Force Linux/Unix environment compliance as per Boot Prompt
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)

TesserCPU *g_cpu_context = NULL;

// Helpers de Pilha
void stack_push(TesserCPU *cpu, uint16_t val) {
    if (cpu->sp < 16) {
        cpu->stack[cpu->sp++] = val;
    } else {
        printf("[CPU ERROR] Stack Overflow!\n");
    }
}

uint16_t stack_pop(TesserCPU *cpu) {
    if (cpu->sp > 0) {
        return cpu->stack[--cpu->sp];
    } else {
        printf("[CPU ERROR] Stack Underflow!\n");
        return 0;
    }
}

void cpu_step(TesserCPU *cpu) {
    g_cpu_context = cpu;

    // Fetch Opcode
    uint8_t opcode = bus_read(cpu->pc);
    cpu->pc++;

    switch (opcode) {
        case OP_PUSH: // 0x01 imm16
        {
            uint8_t hi = bus_read(cpu->pc++);
            uint8_t lo = bus_read(cpu->pc++);
            uint16_t val = (hi << 8) | lo;
            stack_push(cpu, val);
            break;
        }

        case OP_MUI_SET: // 0x02 id8
        {
            uint8_t id = bus_read(cpu->pc++);
            uint16_t val = stack_pop(cpu);
            
            // Telemetria
            last_mui_id = id;
            last_mui_val = val;
            
            printf("[HW IO] MUI_SET [ID: %d] = %d\n", id, val);
            break;
        }

        case OP_WAIT: // 0x03
        {
            uint16_t ms = stack_pop(cpu);
            printf("[HW IO] WAIT %d ms\n", ms);
            SLEEP_MS(ms);
            break;
        }

        case OP_JMP: // 0x04 addr16
        {
            uint8_t hi = bus_read(cpu->pc++);
            uint8_t lo = bus_read(cpu->pc++);
            uint16_t addr = (hi << 8) | lo;
            cpu->pc = addr;
            break;
        }
        
        case OP_SMOOTH: // 0x05 id8
        {
            uint8_t id = bus_read(cpu->pc++);
            uint16_t val = stack_pop(cpu);
            last_mui_id = id;
            last_mui_val = val;
            // printf("[HW IO] SMOOTH [ID: %d] = %d\n", id, val);
            break;
        }
        
        case OP_MUI_GET: // 0x06 id8
        {
            bus_read(cpu->pc++); // Consome ID
            // Simulação de sensor: valor fixo ou randômico
            uint16_t sensor_val = 123; 
            stack_push(cpu, sensor_val);
            break;
        }
        
        case OP_SUB: // 0x07
        {
            uint16_t b = stack_pop(cpu);
            uint16_t a = stack_pop(cpu);
            stack_push(cpu, a - b);
            break;
        }
        
        case OP_JMP_POS: // 0x08 addr16
        {
            uint8_t hi = bus_read(cpu->pc++);
            uint8_t lo = bus_read(cpu->pc++);
            uint16_t addr = (hi << 8) | lo;
            
            uint16_t a = stack_pop(cpu);
            if ((int16_t)a > 0) {
                cpu->pc = addr;
            }
            break;
        }

        default:
            printf("[CPU WARN] Unknown Opcode: 0x%02X at PC: %d\n", opcode, cpu->pc-1);
            break;
    }
}

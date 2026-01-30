#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h"
#include <stdio.h>
#include <unistd.h>

// Fixed sleep for visualization as per Master Prompt
#define VISUAL_SLEEP_MS 200

TesserCPU *g_cpu_context = NULL;

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

    if (cpu->pc >= 65535) {
        cpu->pc = 0; // Wrap around safety
    }

    // Fetch Opcode
    uint8_t opcode = bus_read(cpu->pc);
    cpu->pc++;

    switch (opcode) {
        case OP_PUSH: // 0x01 imm16
        {
            uint8_t hi = bus_read(cpu->pc++);
            uint8_t lo = bus_read(cpu->pc++);
            uint16_t val = (hi << 8) | lo;
            
            // Stack Protection (Requested Step 3)
            if (cpu->sp >= 16) {
                cpu->sp = 15; // Cap at max
                printf("[CPU ERR] Stack Overflow Prevented!\n");
            }
            
            stack_push(cpu, val);
            break;
        }

        case OP_MUI_SET: // 0x02 id8 (Pop val)
        {
            uint8_t id = bus_read(cpu->pc++);
            uint16_t val = stack_pop(cpu);
            
            // Telemetria & Logging
            last_mui_id = id;
            last_mui_val = val;
            
            // Format requested: [HW IO] MUI_SET ID:%d VAL:%d
            printf("[HW IO] MUI_SET ID:%d VAL:%d\n", id, val);
            break;
        }

        case OP_WAIT: // 0x03
        {
            // Verilog "Truth": Hardware is No-op/Fast.
            // Emulator: Fixed pause for visualization.
            // Note: Does NOT pop from stack, as per Verilog "No-op" analysis.
            usleep(VISUAL_SLEEP_MS * 1000);
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
        
        case OP_JMP_POS: // 0x08 addr16 (Conditional Jump if Pop > 0)
        {
             uint8_t hi = bus_read(cpu->pc++);
             uint8_t lo = bus_read(cpu->pc++);
             uint16_t addr = (hi << 8) | lo;
             
             uint16_t val = stack_pop(cpu);
             if ((int16_t)val > 0) {
                 cpu->pc = addr;
             }
             break;
        }
        
        // Keeping other opcodes as pass-through or basic implementation if they exist in header
        // to avoid compile errors if firmware uses them, but focusing on the requested set.
        case 0xFF: // HALT or typical end
            usleep(100000);
            break;

        default:
            // Optional: Handle unknown or other opcodes if necessary
            // For strict compliance to "Transplant", we only strictly ensure the 4 above.
            break;
    }
}

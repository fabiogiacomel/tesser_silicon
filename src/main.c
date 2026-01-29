#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h" 

// Flag global para modo de teste
int g_test_mode = 0;

// Microkernel de Emergência (Pisca LED) - Opcode Stack Machine
// PUSH 1000, MUI_SET 0, WAIT, PUSH 0, MUI_SET 0, WAIT, JMP 0
uint8_t emergency_kernel[] = {
    // 0: PUSH 1000 (Valor LED)
    0x01, 0x03, 0xE8,       
    // 3: MUI_SET 0 (Consome 1000)
    0x02, 0x00,             
    // 5: PUSH 20 (Tempo ms)
    0x01, 0x00, 0x14,       
    // 8: WAIT (Consome 20)
    0x03,                   
    // 9: PUSH 0 (Valor LED)
    0x01, 0x00, 0x00,       
    // 12: MUI_SET 0
    0x02, 0x00,             
    // 14: PUSH 20 (Tempo ms)
    0x01, 0x00, 0x14,       
    // 17: WAIT
    0x03,                   
    // 18: JMP 0000
    0x04, 0x00, 0x00        
};

void load_firmware() {
    FILE *f = fopen("firmware.hex", "r");
    if (f) {
        if (!g_test_mode) printf("[BOOT] Loading firmware.hex...\n");
        // Leitura de texto hex
        char line[64];
        uint16_t addr = 0;
        while(fgets(line, sizeof(line), f)) {
            if(strlen(line) < 2) continue;
            unsigned int byte_val;
            if(sscanf(line, "%x", &byte_val) == 1) {
                bus_write(addr++, (uint8_t)byte_val);
            }
        }
        fclose(f);
    } else {
        if (!g_test_mode) printf("[BOOT WARN] firmware.hex not found! Injecting Emergency Microkernel.\n");
        for (int i = 0; i < sizeof(emergency_kernel); i++) {
            bus_write(i, emergency_kernel[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    // Verificação de Argumentos
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        g_test_mode = 1;
        printf("[TEST MODE] Running Smoke Test (100 Cycles)...\n");
    }

    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    
    // Configura contexto global
    extern TesserCPU *g_cpu_context;
    g_cpu_context = &cpu;

    load_firmware();

    if (!g_test_mode) printf("[SYSTEM] Tesser Silicon Emulator Started (Stack Machine Mode).\n");

    int cycles = 0;
    int max_cycles = g_test_mode ? 100 : -1; // -1 = Infinito

    while (1) {
        cpu_step(&cpu);
        
        if (!g_test_mode) {
            dump_json_state();
        }

        // Critério de Parada do Teste
        if (g_test_mode) {
            cycles++;
            if (cycles >= max_cycles) {
                printf("[TEST SUCCESS] Executed %d cycles without crash.\n", cycles);
                // Verificação extra: O PC moveu?
                if (cpu.pc == 0 && cycles > 5) {
                    printf("[TEST FAIL] PC Stuck at 0 (Did you load firmware?)!\n");
                    return 1;
                }
                return 0; // Sucesso
            }
        }
    }

    return 0;
}

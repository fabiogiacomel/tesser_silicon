#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h" 

int g_test_mode = 0;

void load_firmware() {
    FILE *f = fopen("firmware.hex", "r");
    if (f) {
        if (!g_test_mode) printf("[BOOT] Loading firmware.hex...\n");
        char line[64];
        uint16_t addr = 0;
        while(fgets(line, sizeof(line), f)) {
            // Ignora linhas vazias ou curtas
            if(strlen(line) < 2) continue;
            unsigned int byte_val;
            // Lê byte hex numérico
            if(sscanf(line, "%x", &byte_val) == 1) {
                bus_write(addr++, (uint8_t)byte_val);
            }
        }
        fclose(f);
    } else {
        printf("[BOOT ERROR] firmware.hex not found! Cannot boot.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // Modo de teste opcional
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        g_test_mode = 1;
        printf("[TEST MODE] Running Smoke Test...\n");
    }

    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    
    // Set global context
    g_cpu_context = &cpu;

    load_firmware();

    if (!g_test_mode) printf("[SYSTEM] Tesser Silicon Emulator Started (Stack Machine).\n");

    int cycles = 0;
    while (1) {
        cpu_step(&cpu);
        
        if (!g_test_mode) {
            dump_json_state();
        } else {
            cycles++;
            if (cycles > 100) break;
        }
    }
    
    if (g_test_mode) printf("[TEST SUCCESS] Grid executed.\n");

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h" 

// Fallback ROM to generate a heartbeat if firmware.hex is missing
// Logic:
// LOOP:
//   PUSH 1023
//   MUI_SET 0
//   WAIT
//   PUSH 0
//   MUI_SET 0
//   WAIT
//   JMP LOOP

void load_fallback_rom() {
    printf("[BOOT WARN] firmware.hex not found. Loading Hardcoded ROM (Heartbeat)...\n");
    
    uint16_t a = 0;
    
    // PUSH 1000
    bus_write(a++, OP_PUSH);
    bus_write(a++, 0x03); // Hi 1000 (0x03E8)
    bus_write(a++, 0xE8); // Lo
    
    // MUI_SET 0 (Pop 1000)
    bus_write(a++, OP_MUI_SET);
    bus_write(a++, 0x00); // ID 0
    
    // WAIT (Fixed pause)
    bus_write(a++, OP_WAIT);
    
    // PUSH 0
    bus_write(a++, OP_PUSH);
    bus_write(a++, 0x00);
    bus_write(a++, 0x00);
    
    // MUI_SET 0 (Pop 0)
    bus_write(a++, OP_MUI_SET);
    bus_write(a++, 0x00); // ID 0
    
    // WAIT
    bus_write(a++, OP_WAIT);
    
    // JMP 0x0000
    bus_write(a++, OP_JMP);
    bus_write(a++, 0x00);
    bus_write(a++, 0x00);
}

void load_firmware() {
    FILE *f = fopen("firmware.hex", "r");
    if (f) {
        printf("[BOOT] Loading firmware.hex...\n");
        char line[64];
        uint16_t addr = 0;
        int count = 0;
        while(fgets(line, sizeof(line), f)) {
            if(strlen(line) < 2) continue;
            unsigned int byte_val;
            if(sscanf(line, "%x", &byte_val) == 1) {
                bus_write(addr++, (uint8_t)byte_val);
                count++;
            }
        }
        fclose(f);
        printf("[BOOT] %d bytes loaded.\n", count);
    } else {
        load_fallback_rom();
    }
}

int g_debug_mode = 0;

int main(int argc, char *argv[]) {
    // Check for --debug flag
    if (argc > 1 && strcmp(argv[1], "--debug") == 0) {
        g_debug_mode = 1;
        // Turn off buffering for proper pipe interaction
        setvbuf(stdout, NULL, _IOLBF, 0);
        setvbuf(stdin, NULL, _IOLBF, 0);
        printf("[DEBUG MODE] Stepping enabled. Waiting for input...\n");
    }

    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    g_cpu_context = &cpu;

    load_firmware();

    printf("[SYSTEM] Tesser Silicon Emulator Started (Stack Machine).\n");
    
    // Initial Dump for Debugger to catch PC=0 before first step
    if(g_debug_mode) dump_json_state();

    while (1) {
        if (g_debug_mode) {
             // Wait for "Step" command (newline) from Controller
             getchar();
        }
        
        cpu_step(&cpu);
        dump_json_state();
    }
    
    return 0;
}

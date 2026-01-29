#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_telemetry.h"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)
#endif

// Loader com Fallback
void load_firmware_or_fallback(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("WARNING: '%s' not found. Injecting 'Hardcoded Blink' firmware.\n", filename);
        
        // Injetar Programa de Teste (PISCA LED - Stack Machine)
        // 0: PUSH 1000
        // 3: MUI_SET 0
        // 5: PUSH 500
        // 8: WAIT
        // 9: PUSH 0
        // 12: MUI_SET 0
        // 14: PUSH 500
        // 17: WAIT
        // 18: JMP 0
        
        uint16_t ptr = 0;
        
        // PUSH 1000
        bus_write(ptr++, OP_PUSH); bus_write(ptr++, 0x03); bus_write(ptr++, 0xE8);
        // MUI_SET 0 (LED ON)
        bus_write(ptr++, OP_MUI_SET); bus_write(ptr++, 0x00);
        // PUSH 500
        bus_write(ptr++, OP_PUSH); bus_write(ptr++, 0x01); bus_write(ptr++, 0xF4);
        // WAIT
        bus_write(ptr++, OP_WAIT);
        
        // PUSH 0
        bus_write(ptr++, OP_PUSH); bus_write(ptr++, 0x00); bus_write(ptr++, 0x00);
        // MUI_SET 0 (LED OFF)
        bus_write(ptr++, OP_MUI_SET); bus_write(ptr++, 0x00);
        // PUSH 500
        bus_write(ptr++, OP_PUSH); bus_write(ptr++, 0x01); bus_write(ptr++, 0xF4);
        // WAIT
        bus_write(ptr++, OP_WAIT);
        
        // JMP 0 (Loop)
        bus_write(ptr++, OP_JMP); bus_write(ptr++, 0x00); bus_write(ptr++, 0x00);
        
        return;
    }
    
    // Se arquivo existe, carrega
    char line[64];
    uint16_t addr = 0;
    while(fgets(line, sizeof(line), fp)) {
        if(strlen(line) < 2) continue;
        unsigned int byte_val;
        if(sscanf(line, "%x", &byte_val) == 1) {
            bus_write(addr++, (uint8_t)byte_val);
        }
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    // printf("--- Tesser Silicon Stack VM (Watchtower Mode) ---\n");

    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    
    // Configura contexto global
    extern TesserCPU *g_cpu_context;
    g_cpu_context = &cpu;
    
    int test_mode = 0;
    const char* filename = "firmware.hex";

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "--smoke") == 0) {
            test_mode = 1;
        } else {
            filename = argv[i];
        }
    }

    load_firmware_or_fallback(filename);
    
    long cycles = 0;
    
    // Infinite Loop for Watchtower Server (or limited for Test)
    while (1) {
        cpu_step(&cpu);
        
        if (!test_mode) {
            dump_json_state();
            SLEEP_MS(50);
        } else {
            cycles++;
            if (cycles >= 100) break;
        }
    }
    
    if (test_mode) {
        printf("[TEST] Simulation completed 100 cycles successfully.\n");
    }
    
    return 0;
}

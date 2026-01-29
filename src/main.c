#include <stdio.h>
#include <string.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"
#include "tesser_peripherals.h"
#include "tesser_memory.h"
#include "tesser_telemetry.h"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)
#endif

int main() {
    // Silence initial prints to keep JSON stream clean or just accept them as ignored lines
    // printf("--- Tesser Silicon Watchtower Demo ---\n");

    // 1. Inicialize Sistema
    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.current_privilege_level = PRIV_KERNEL;
    g_cpu_context = &cpu;

    peripherals_init();
    memory_pool_init();
    
    // 2. Load Program mimicking complex behavior
    uint16_t prog_ptr = 0;

    // Loop Program:
    // 0x00: MOV R0, 0x01
    // 0x04: MOV R1, 0x02
    // 0x08: ADD R0, R1
    // ... trigger AES ...
    
    // Simply put: valid instructions in loop
    // 0: MOV R0, 10
    bus_write(prog_ptr++, 0x01); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x0A);
    // 4: MOV R1, 20
    bus_write(prog_ptr++, 0x01); bus_write(prog_ptr++, 0x01); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x14);
    // 8: ADD R0, R1 (R0=30)
    bus_write(prog_ptr++, 0x02); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x01);
    
    // Trigger AES (Store at 0xF000)
    // MOV R2, 1 (Start)
    bus_write(prog_ptr++, 0x01); bus_write(prog_ptr++, 0x02); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x01);
    // STORE R2 to CTRL
    bus_write(prog_ptr++, 0x03); bus_write(prog_ptr++, 0xF0); bus_write(prog_ptr++, 0x00); bus_write(prog_ptr++, 0x02);
    
    int pc_limit = prog_ptr;
    cpu.pc = 0;

    // Infinite Loop for Visualization
    while (1) {
        // Step Hardware
        peripherals_tick();
        
        // Step CPU (only if within program range, else reset)
        if (cpu.pc < pc_limit) {
            cpu_step(&cpu);
        } else {
            // Reset for demo loop
             cpu.pc = 0;
             // Reset AES for fun?
             // peripherals_init(); // optional
        }

        // Telemetry
        dump_json_state();
        
        // Slow down for visualizer
        SLEEP_MS(500);
    }
    
    return 0;
}

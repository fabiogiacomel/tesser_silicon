#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tesser_cpu.h"
#include "tesser_bus.h"

// Função para Carregar Firmware Externo
void load_program_from_hex(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("CRITICAL EXCEPTION: Firmware file '%s' not found.\n", filename);
        printf("Hint: Run the Assembler first (menu option 6) or check file path.\n");
        exit(1);
    }

    uint16_t addr = 0;
    char line[32];
    
    printf("[LOADER] Reading %s...\n", filename);
    
    while (fgets(line, sizeof(line), fp)) {
        // Ignorar linhas vazias ou muito curtas
        if (strlen(line) < 2) continue;
        
        unsigned int byte_val;
        // Lê hex da linha
        if (sscanf(line, "%x", &byte_val) == 1) {
            bus_write(addr, (uint8_t)byte_val);
            addr++;
        }
    }
    
    fclose(fp);
    printf("[LOADER] Success. %d bytes loaded into RAM.\n", addr);
}

int main(int argc, char *argv[]) {
    // 1. Instanciar CPU
    TesserCPU cpu;
    memset(&cpu, 0, sizeof(cpu));
    
    // Configura contexto global (caso telemetria seja linkada)
    extern TesserCPU *g_cpu_context;
    g_cpu_context = &cpu;
    
    // 2. Carregar Programa
    // Se passado argumento, usa ele, senão usa padrão "firmware.hex"
    const char* fw_file = "firmware.hex";
    if (argc > 1) fw_file = argv[1];
    
    load_program_from_hex(fw_file);
    
    // 3. Execução
    printf("Starting CPU Execution...\n");
    
    // Executa um número razoável de ciclos para ver o programa rodar
    // Como o pisca.tasm é um loop, 50 ciclos devem mostrar atividade
    int max_cycles = 50;
    
    for (int i = 0; i < max_cycles; i++) {
        cpu_step(&cpu);
        // Opcional: Imprimir estado a cada passo para debug
        // printf("PC: %04X | R0: %04X\n", cpu.pc, cpu.regs[0]);
    }
    
    printf("\n=== Execution Paused after %d cycles ===\n", max_cycles);
    printf("Final PC: 0x%04X\n", cpu.pc);
    printf("R0: 0x%04X  R1: 0x%04X\n", cpu.regs[0], cpu.regs[1]);
    
    return 0;
}

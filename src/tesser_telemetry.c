#include <stdio.h>
#include "tesser_cpu.h"
#include "tesser_telemetry.h"

// Variáveis de estado para visualização de IO
int last_mui_id = -1;
int last_mui_val = 0;

void dump_json_state() {
    if (g_cpu_context == NULL) return;

    // Start JSON object
    printf("{\"pc\": %d, \"sp\": %d, \"stack\": [", g_cpu_context->pc, g_cpu_context->sp);
    
    // Dump Stack (All 16 positions)
    for (int i = 0; i < 16; i++) {
        printf("%d", g_cpu_context->stack[i]);
        if (i < 15) printf(", ");
    }
    printf("], ");

    // Dump MUI Status with requested keys "mui_id" and "mui_val"
    printf("\"mui_id\": %d, ", last_mui_id);
    printf("\"mui_val\": %d", last_mui_val);

    printf("}\n"); // End JSON object and flush line
    fflush(stdout);
}

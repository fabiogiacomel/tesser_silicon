#include <stdio.h>
#include "tesser_cpu.h"
#include "tesser_peripherals.h"

void dump_json_state() {
    if (g_cpu_context == NULL) return;

    // Start JSON object
    printf("{\"pc\": %d, \"regs\": [", g_cpu_context->pc);
    
    // Dump Registers
    for (int i = 0; i < 16; i++) {
        printf("%d", g_cpu_context->regs[i]);
        if (i < 15) printf(", ");
    }
    printf("], ");

    // Dump AES Status
    uint8_t aes_ctrl = mmio_read(AES_CONTROL);
    printf("\"aes_status\": ");
    if (aes_ctrl & AES_CTRL_READY) {
        printf("\"READY\"");
    } else {
        // Need to check specific implementation details or shadow state in separate var if we want "BUSY" vs "IDLE" accurately
        // But based on previous code: if start bit was set and ready is low, it's BUSY or IDLE.
        // Let's rely on the bit check from logic.
        // Checking MMIO 'read' doesn't easily give "BUSY" boolean without checking internal state which is static in peripherals.c
        // For simplicity, we will infer or just print raw value/simple logic.
        // If we want "BUSY", we might need to expose is_busy from peripherals or infer from internal state if accessible.
        // Since `aes` struct is static in `tesser_peripherals.c`, we can't read `is_busy` directly here.
        // We will just print the Control Register Value for now, or modify peripherals to export status.
        // Let's modify peripherals.c to be cleaner later, but for now:
        printf("\"%s\"", (aes_ctrl & AES_CTRL_READY) ? "READY" : "BUSY/IDLE");
    }

    printf("}\n"); // End JSON object and flush line
    fflush(stdout);
}

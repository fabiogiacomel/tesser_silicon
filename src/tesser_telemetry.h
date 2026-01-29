#ifndef TESSER_TELEMETRY_H
#define TESSER_TELEMETRY_H

#include <stdint.h>

// Variáveis globais para rastrear último estado de IO (MUI)
// Definidas aqui como extern, implementadas em tesser_telemetry.c
extern int last_mui_id;
extern int last_mui_val;

void dump_json_state();

#endif // TESSER_TELEMETRY_H

/* * TESSER SILICON v1.0 - GOLD MASTER RTL
 * Architecture: 16-bit Stack Machine
 * Target: FPGA / ASIC
 * Author: Commander & Antigravity
 */

module tesser_core (
    input wire clk,             // Clock (ex: 50MHz)
    input wire rst_n,           // Reset (Active Low)
    output reg [7:0] mui_id,    // Endereço do Dispositivo (IO)
    output reg [15:0] mui_val,  // Valor do Dado (IO)
    output reg mui_wr           // Strobe de Escrita (Trigger)
);

    // --- PARÂMETROS DE HARDWARE ---
    parameter STACK_DEPTH = 16;
    parameter WIDTH = 16;
    
    // Constante para o WAIT (Simulando 200ms em 50MHz -> 10.000.000 ciclos)
    parameter WAIT_CYCLES = 32'd10000000; 

    // --- OPCODES (ISA) ---
    localparam OP_PUSH    = 8'h01;
    localparam OP_MUI_SET = 8'h02;
    localparam OP_WAIT    = 8'h03;
    localparam OP_JMP     = 8'h04;
    localparam OP_SUB     = 8'h07;
    localparam OP_JMP_POS = 8'h08;

    // --- REGISTRADORES INTERNOS ---
    reg [WIDTH-1:0] pc;                 // Program Counter
    reg [WIDTH-1:0] sp;                 // Stack Pointer
    reg [WIDTH-1:0] stack [0:STACK_DEPTH-1]; // A Pilha (RAM Distribuída)
    reg [31:0]      wait_counter;       // Contador para OP_WAIT

    // --- ESTADOS DA FSM ---
    localparam S_FETCH   = 2'd0;
    localparam S_DECODE  = 2'd1;
    localparam S_EXECUTE = 2'd2;
    localparam S_WAITING = 2'd3;
    
    reg [1:0] state;
    
    // Variáveis temporárias para Fetch
    reg [23:0] instruction_raw; // Instrução crua (Opcode + Arg)
    reg [7:0]  opcode;
    reg [15:0] arg;

    // --- MEMÓRIA DE PROGRAMA (ROM) ---
    // Na prática, isso seria um bloco de memória separado.
    reg [23:0] rom [0:63]; 

    initial begin
        // PROGRAMA: METRÔNOMO (Pisca 100 -> 255 -> 0)
        // Formato: {8hOpcode, 16hArg}
        rom[0] = {OP_PUSH,    16'd100};
        rom[1] = {OP_MUI_SET, 16'd0};   // ID 0
        rom[2] = {OP_WAIT,    16'd0};
        
        rom[3] = {OP_PUSH,    16'd255};
        rom[4] = {OP_MUI_SET, 16'd0};
        rom[5] = {OP_WAIT,    16'd0};
        
        rom[6] = {OP_PUSH,    16'd0};
        rom[7] = {OP_MUI_SET, 16'd0};
        rom[8] = {OP_WAIT,    16'd0};
        
        rom[9] = {OP_JMP,     16'd0}; // Loop START
    end

    // --- LÓGICA PRINCIPAL ---
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= S_FETCH;
            pc <= 0;
            sp <= 0;
            mui_wr <= 0;
            wait_counter <= 0;
        end else begin
            // Default reset de sinais pulsados
            mui_wr <= 0; 

            case (state)
                // 1. FETCH
                S_FETCH: begin
                    instruction_raw <= rom[pc];
                    state <= S_DECODE;
                end

                // 2. DECODE
                S_DECODE: begin
                    opcode <= instruction_raw[23:16];
                    arg    <= instruction_raw[15:0];
                    state  <= S_EXECUTE;
                end

                // 3. EXECUTE
                S_EXECUTE: begin
                    pc <= pc + 1; // Incremento Padrão

                    case (opcode)
                        OP_PUSH: begin
                            stack[sp] <= arg;
                            sp <= sp + 1;
                            state <= S_FETCH;
                        end

                        OP_MUI_SET: begin
                            if (sp > 0) begin
                                mui_id <= arg[7:0]; 
                                mui_val <= stack[sp-1]; 
                                mui_wr <= 1; 
                                sp <= sp - 1; 
                            end
                            state <= S_FETCH;
                        end

                        OP_SUB: begin
                            if (sp >= 2) begin
                                stack[sp-2] <= stack[sp-2] - stack[sp-1];
                                sp <= sp - 1;
                            end
                            state <= S_FETCH;
                        end

                        OP_JMP: begin
                            pc <= arg; 
                            state <= S_FETCH;
                        end

                        OP_JMP_POS: begin
                            if (sp > 0) begin
                                sp <= sp - 1; 
                                if ($signed(stack[sp-1]) > 0) begin
                                    pc <= arg;
                                end
                            end
                            state <= S_FETCH;
                        end

                        OP_WAIT: begin
                            wait_counter <= 0;
                            state <= S_WAITING;
                            pc <= pc; // Trava PC
                        end
                        
                        default: state <= S_FETCH; // NOP
                    endcase
                end

                // 4. WAIT
                S_WAITING: begin
                    if (wait_counter >= WAIT_CYCLES) begin
                        state <= S_FETCH;
                    end else begin
                        wait_counter <= wait_counter + 1;
                        pc <= pc; 
                    end
                end
            endcase
        end
    end

endmodule

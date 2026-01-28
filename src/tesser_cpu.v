module tesser_cpu (
    input wire clk,
    input wire reset,
    
    // MUI Interface
    output reg [7:0]  mui_id,
    output reg [15:0] mui_val,
    output reg        mui_we,
    
    // MUI Input Interface
    input wire [15:0] mui_rx_val,
    output reg        mui_re
);

    //-------------------------------------------------------------------------
    // Parameters & Opcodes
    //-------------------------------------------------------------------------
    // Opcodes
    localparam OP_PUSH    = 8'h01;
    localparam OP_MUI_SET = 8'h02;
    localparam OP_WAIT    = 8'h03;
    localparam OP_JMP     = 8'h04;
    localparam OP_SMOOTH  = 8'h05;
    localparam OP_MUI_GET = 8'h06;
    localparam OP_SUB     = 8'h07;
    localparam OP_JMP_POS = 8'h08;

    // FSM States
    localparam STATE_FETCH    = 3'd0;
    localparam STATE_DECODE_1 = 3'd1; // Get first operand byte if needed
    localparam STATE_DECODE_2 = 3'd2; // Get second operand byte if needed
    localparam STATE_EXECUTE  = 3'd3;

    //-------------------------------------------------------------------------
    // Internal Registers & Memory
    //-------------------------------------------------------------------------
    reg [2:0] state;
    
    reg [15:0] pc;          // Program Counter
    reg [7:0]  sp;          // Stack Pointer
    reg [7:0]  opcode;      // Current Opcode
    
    // Operands storage buffer
    reg [15:0] arg_16;      // Determine 16-bit argument (Address or Immediate)
    reg [7:0]  arg_8;       // Determine 8-bit argument (ID)

    // Memories
    reg [15:0] stack [0:15]; // Stack memory: 16 words of 16-bit
    reg [7:0]  rom   [0:255]; // Program memory: 256 bytes

    //-------------------------------------------------------------------------
    // Bootloader / Initial Block
    //-------------------------------------------------------------------------
    integer i;
    initial begin
        // Initialize Regs
        pc = 0;
        sp = 0;
        state = STATE_FETCH;
        mui_we = 0;
        mui_id = 0;
        mui_val = 0;
        mui_re = 0;

        // Clear Stack & ROM
        for (i = 0; i < 16; i = i + 1) stack[i] = 16'h0000;
        for (i = 0; i < 256; i = i + 1) rom[i] = 8'h00;

        // -----------------------------------------------------------------
        // Program: External Firmware
        // Logic: Loaded from firmware.hex
        // -----------------------------------------------------------------
        $readmemh("firmware.hex", rom);
    end

    //-------------------------------------------------------------------------
    // Main FSM & Datapath
    //-------------------------------------------------------------------------
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state    <= STATE_FETCH;
            pc       <= 0;
            sp       <= 0;
            mui_we   <= 0;
            mui_id   <= 0;
            mui_val  <= 0;
            opcode   <= 0;
            arg_16   <= 0;
            arg_8    <= 0;
            mui_re   <= 0;
        end else begin
            // Default signals
            mui_we <= 0; // Strobe style write enable
            // mui_re <= 0; // Dont assume default 0, control strictly logic

            case (state)
                // ------------------------------------------------------------
                // FETCH: Read Opcode
                // ------------------------------------------------------------
                STATE_FETCH: begin
                    opcode <= rom[pc];
                    pc     <= pc + 1;
                    state  <= STATE_DECODE_1;
                    mui_re <= 0; // Safe default
                end

                // ------------------------------------------------------------
                // DECODE 1: Check opcode, fetch 1st operand byte if necessary
                // ------------------------------------------------------------
                STATE_DECODE_1: begin
                    case (opcode)
                        OP_PUSH, OP_JMP, OP_JMP_POS: begin
                            // 16-bit argument: Read MSB
                            arg_16[15:8] <= rom[pc];
                            pc           <= pc + 1;
                            state        <= STATE_DECODE_2;
                        end

                        OP_MUI_SET: begin
                            // 8-bit argument: Read ID
                            arg_8 <= rom[pc];
                            pc    <= pc + 1;
                            state <= STATE_EXECUTE;
                        end

                        OP_MUI_GET: begin
                            // 8-bit argument: ID
                            arg_8  <= rom[pc];
                            // Also set it on bus immediately for lookup
                            mui_id <= rom[pc];
                            mui_re <= 1; // ASSERT READ ENABLE
                            pc     <= pc + 1;
                            state  <= STATE_EXECUTE;
                        end

                        default: begin
                            // No arguments (WAIT, SMOOTH, SUB, etc.)
                            state <= STATE_EXECUTE;
                        end
                    endcase
                end

                // ------------------------------------------------------------
                // DECODE 2: Fetch 2nd operand byte for 16-bit args
                // ------------------------------------------------------------
                STATE_DECODE_2: begin
                    arg_16[7:0] <= rom[pc];
                    pc          <= pc + 1;
                    state       <= STATE_EXECUTE;
                end

                // ------------------------------------------------------------
                // EXECUTE: Perform Operation
                // ------------------------------------------------------------
                STATE_EXECUTE: begin
                    case (opcode)
                        // --- Operations ---
                        OP_PUSH: begin
                            stack[sp] <= arg_16;
                            sp        <= sp + 1;
                        end

                        OP_MUI_SET: begin
                            if (sp > 0) begin
                                mui_id  <= arg_8;
                                mui_val <= stack[sp-1];
                                mui_we  <= 1;
                                sp      <= sp - 1;
                            end
                        end
                        
                        OP_WAIT: begin
                            // No-op
                        end
                        
                        OP_JMP: begin
                            pc <= arg_16;
                        end
                        
                        OP_SMOOTH: begin
                            // No-op
                        end
                        
                        OP_MUI_GET: begin
                            // Read value from input bus
                            stack[sp] <= mui_rx_val;
                            sp        <= sp + 1;
                            mui_re    <= 0; // Deassert
                        end
                        
                        OP_SUB: begin
                            if (sp >= 2) begin
                                stack[sp-2] <= stack[sp-2] - stack[sp-1];
                                sp          <= sp - 1;
                            end
                        end
                        
                        OP_JMP_POS: begin
                             if (sp > 0) begin
                                 if ($signed(stack[sp-1]) > 0) begin
                                     pc <= arg_16;
                                 end
                                 sp <= sp - 1;
                             end
                        end
                    endcase
                    
                    state <= STATE_FETCH;
                end
            endcase
        end
    end

endmodule

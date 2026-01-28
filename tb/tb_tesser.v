`timescale 1ns / 1ps

module tb_tesser;

    // Parameters
    localparam CLK_PERIOD = 10; // 100 MHz

    // Signals
    reg clk;
    reg reset;
    wire [7:0] mui_id;
    wire [15:0] mui_val;
    wire mui_we;
    wire mui_re;         // New
    reg [15:0] mui_rx_val; // New

    // DUT Instantiation
    tesser_cpu uut (
        .clk(clk),
        .reset(reset),
        .mui_id(mui_id),
        .mui_val(mui_val),
        .mui_we(mui_we),
        .mui_re(mui_re),         // New
        .mui_rx_val(mui_rx_val)  // New
    );

    // Sensor Simulation Logic
    always @(*) begin
        if (mui_re && (mui_id == 8'h03)) begin
            mui_rx_val = 16'd1000;
        end else begin
            mui_rx_val = 16'd0;
        end
    end

    // Clock Generation
    initial begin
        clk = 0;
        forever #(CLK_PERIOD/2) clk = ~clk;
    end

    // Simulation & Stimulus
    initial begin
        // VCD Dump Setup
        $dumpfile("sim/tesser.vcd");
        $dumpvars(0, tb_tesser);

        // Reset Sequence
        $display("SIM: Starting Simulation (Prompt 7 Timer Test)...");
        reset = 1;
        #20;
        reset = 0;
        $display("SIM: Reset Deasserted.");

        // Wait for simulation end
        #2000; // Increased to allow input logic to process
        $display("SIM: Timeout Reached. Finishing.");
        $finish;
    end

    // Monitor Output
    always @(posedge clk) begin
        if (mui_we) begin
            $display("TIME %t | MUI_WRITE: ID=0x%h VAL=%d (0x%h)", $time, mui_id, mui_val, mui_val);
        end
        if (mui_re) begin
            $display("TIME %t | MUI_READ:  ID=0x%h. Sensor sends %d", $time, mui_id, mui_rx_val);
        end
    end

endmodule

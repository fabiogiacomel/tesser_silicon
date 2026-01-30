`timescale 1ns / 1ps

module tesser_tb;

    // Entradas
    reg clk;
    reg rst_n;

    // Saídas
    wire [7:0] mui_id;
    wire [15:0] mui_val;
    wire mui_wr;

    // Instancia o DUT com ciclo de espera rápido para simulação
    tesser_core #(.WAIT_CYCLES(5)) dut (
        .clk(clk),
        .rst_n(rst_n),
        .mui_id(mui_id),
        .mui_val(mui_val),
        .mui_wr(mui_wr)
    );

    // Clock: 50MHz (Period 20ns)
    always #10 clk = ~clk;

    initial begin
        // Reset
        clk = 0;
        rst_n = 0;
        
        $dumpfile("tesser_wave.vcd");
        $dumpvars(0, tesser_tb);

        #100;
        rst_n = 1;
        
        $display("--- TESSER SILICON HARDWARE SIMULATION START ---");
        // Monitora apenas quando há escrita no barramento de IO
        $monitor("Time: %t | PC: %2d | Op: %h | StackTop: %d | IO OUT -> ID:%d Val:%d (WR:%b)", 
                 $time, dut.pc, dut.opcode, dut.stack[dut.sp > 0 ? dut.sp-1 : 0], mui_id, mui_val, mui_wr);

        // Roda simulação suficiente para alguns ciclos do Metrônomo
        #2000;
        
        $display("--- SIMULATION END ---");
        $finish;
    end

endmodule

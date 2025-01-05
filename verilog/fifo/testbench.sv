// Testbench timescale
`timescale 1ns/100ps


// FIFO testbench
module tb_fifo();
    // input
	reg 			clk, reset_n, rd_en, wr_en; // 1 bit input
	reg		[31:0]	d_in;                       // 32 bit input

    // output
	wire	[31:0]	d_out;                                          // 32 bit output
	wire			full, empty, wr_ack, wr_err, rd_ack, rd_err;    // 1 bit output
	wire	[3:0]	data_count, next_data_count;                    // 4 bit output
    parameter       STEP = 10;                                      // STEP for clk

    // flip clock by STEP/2
    always #(STEP/2) clk = ~clk;

    // Declare module to be tested
    // Device Under Test
	fifo DUT(.clk(clk), .reset_n(reset_n), .rd_en(rd_en), .wr_en(wr_en), .d_in(d_in),
            .d_out(d_out), .full(full), .empty(empty), .wr_ack(wr_ack), .wr_err(wr_err), .rd_ack(rd_ack), .rd_err(rd_err), .data_count(data_count), .next_data_count(next_data_count));

    initial
    begin
        // Initialize
        clk = 0; reset_n = 0; rd_en = 0; wr_en = 0; d_in = 32'h00000000;

        // negate reset
        #STEP;  // 10ns
        #2; reset_n = 1; // 12ns

        // EMPTY and Read
        #STEP; // 22ns
        #2; rd_en = 1; //24ns
        #STEP; rd_en = 0; //34ns

        // Write Test
        #STEP; wr_en = 1; d_in = 32'h00000011; //44ns
        #STEP; d_in = 32'h00000022; //55ns
        #STEP; d_in = 32'h00000033; //65ns
        #STEP; d_in = 32'h00000044; //75ns
        #STEP; d_in = 32'h00000055; //85ns
        #STEP; d_in = 32'h00000066; //95ns
        #STEP; d_in = 32'h00000077; //105ns 
        #STEP; d_in = 32'h00000088; //115ns
        #STEP; d_in = 32'h00000099; //125ns
        #STEP; d_in = 32'h000000aa; //135ns

        // Read Test
        #STEP; wr_en = 0; rd_en = 1; d_in = 32'h000000bb; //145ns
        #STEP; wr_en = 1; rd_en = 0; d_in = 32'h000000cc; //155ns
        #STEP; wr_en = 0; rd_en = 1; //165ns
        #STEP; rd_en = 0; //175ns
        #STEP; rd_en = 1; //185ns 
        #80; //265ns
        $finish;
    end

    initial
    begin
      	$dumpfile("dump.vcd");
      	$dumpvars(1);
    end
  
endmodule

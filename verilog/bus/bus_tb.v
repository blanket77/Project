`timescale 1ns / 1ps

module bus_tb;

    reg clk;
    reg reset_n;
    
    reg m0_req, m1_req, m2_req;
    reg m0_wr, m1_wr, m2_wr;
    reg [7:0] m0_address, m1_address, m2_address;
    reg [31:0] m0_dout, m1_dout, m2_dout;

    wire [31:0] s0_dout;
    wire [31:0] s1_dout;
    wire [31:0] s2_dout;
    wire [31:0] s3_dout;
    wire [31:0] s4_dout;
    wire [31:0] s5_dout;
    wire [31:0] s6_dout;
    wire [31:0] s7_dout;

    wire m0_grant, m1_grant, m2_grant;
    wire [31:0] m_din;
    wire [7:0] s_sel, s_address;
    wire s_wr;
    wire [31:0] s_din;

    // DUT instantiation
    bus uut (
        .clk(clk),
        .reset_n(reset_n),
        .m0_req(m0_req), .m1_req(m1_req), .m2_req(m2_req),
        .m0_wr(m0_wr), .m1_wr(m1_wr), .m2_wr(m2_wr),
        .m0_address(m0_address), .m1_address(m1_address), .m2_address(m2_address),
        .m0_dout(m0_dout), .m1_dout(m1_dout), .m2_dout(m2_dout),
        .s0_dout(s0_dout), .s1_dout(s1_dout), .s2_dout(s2_dout), .s3_dout(s3_dout),
        .s4_dout(s4_dout), .s5_dout(s5_dout), .s6_dout(s6_dout), .s7_dout(s7_dout),
        .m0_grant(m0_grant), .m1_grant(m1_grant), .m2_grant(m2_grant),
        .m_din(m_din),
        .s_sel(s_sel), .s_address(s_address), .s_wr(s_wr), .s_din(s_din)
    );

    // Clock generation
    always #5 clk = ~clk; // 100MHz clock (10ns period)

    initial begin
        // Initialize signals
        clk = 0;
        reset_n = 0;
        m0_req = 0; m1_req = 0; m2_req = 0;
        m0_wr = 0;  m1_wr = 0;  m2_wr = 0;
        m0_address = 8'd0; m1_address = 8'd0; m2_address = 8'd0;
        m0_dout = 32'd0; m1_dout = 32'd0; m2_dout = 32'd0;
        
        // Reset the design
        #20 reset_n = 1;  // Release reset after 20ns

        // Scenario 1: M0 requests a write (address in S0 range)
        // Write data 32'hAABBCCDD at address 0x10
        #10 m0_req = 1; m0_wr = 1; m0_address = 8'h10; m0_dout = 32'hAABBCCDD;
        #20 m0_req = 0; m0_wr = 0; // Deassert M0 request after some time

        // Scenario 2: M0 requests a read from the same address to verify write
        #10 m0_req = 1; m0_wr = 0; m0_address = 8'h10;
        #20 m0_req = 0; 
        
        // Check the waveform to ensure m_din = 32'hAABBCCDD when reading back.

        // Scenario 3: M1 tries to request bus after M0 is done
        // Write from M1 to address 0x30 (S1 range)
        #10 m1_req = 1; m1_wr = 1; m1_address = 8'h30; m1_dout = 32'h12345678;
        #20 m1_req = 0; m1_wr = 0;

        // Scenario 4: M2 reads from M1's written address
        #10 m2_req = 1; m2_wr = 0; m2_address = 8'h30;
        #20 m2_req = 0;

        // Scenario 5: Out-of-range address from M2 (no slave selected)
        #10 m2_req = 1; m2_wr = 0; m2_address = 8'hFF; // S7 range is valid, let's try something invalid like 8'hFE for demonstration
        // Actually 0xFF is in S7 range. Let's pick invalid:
        #10 m2_address = 8'hFE; // This is still in S7 range (E0~FF). Let's pick something beyond 8'hFF?
        // The given design only has 0x00~0xFF. Let's assume out-of-range scenario doesn't occur as memory is 256 words.
        // For demonstration only, if we had a bigger address width, we could test out-of-range.
        // We'll just trust if no request or some invalid scenario, s_sel = 8'h00.

        #20 m2_req = 0;

        // Finish simulation
        #100 $stop;
    end

    initial
    begin
      	$dumpfile("dump.vcd");
      	$dumpvars(1);
    end

endmodule

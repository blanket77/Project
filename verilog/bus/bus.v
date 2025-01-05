`timescale 1ns / 100ps

module bus (
    // clock and reset
    input clk,               // clock
    input reset_n,           // reset signal

    input m0_req, m1_req, m2_req,    // master 0, 1, 2 request signal

    input m0_wr, m1_wr, m2_wr,       // master 0, 1, 2 write signal

    input [7:0] m0_address, m1_address, m2_address, // master 0, 1, 2 address

    input [31:0] m0_dout, m1_dout, m2_dout,   // master 0, 1, 2 data out

    output reg [31:0] s0_dout, // slave 0 data out
    output reg [31:0] s1_dout, // slave 1 data out
    output reg [31:0] s2_dout, // slave 2 data out
    output reg [31:0] s3_dout, // slave 3 data out
    output reg [31:0] s4_dout, // slave 4 data out
    output reg [31:0] s5_dout, // slave 5 data out
    output reg [31:0] s6_dout, // slave 6 data out
    output reg [31:0] s7_dout, // slave 7 data out

  
    output reg m0_grant, m1_grant, m2_grant,   // master 0, 1, 2 grant signal

    output reg [31:0] m_din,    // master data in

    output reg [7:0] s_sel,     // slave select signal             
    output reg [7:0] s_address,    // slave address signal
    output reg s_wr,    // slave write signal
    output reg [31:0] s_din     // slave data in 
);

  
    reg [31:0] mem[0:255]; // memory
    integer i; // loop variable

    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin // reset
            for (i = 0; i < 256; i = i + 1) begin // memory clear
                mem[i] <= 32'b0; // memory clear
            end
        end else begin
            if (s_wr && s_sel != 8'h00) begin   // write operation
                
                mem[s_address] <= s_din;// write data
            end
        end
    end

    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            m0_grant <= 0; // master 0 grant
            m1_grant <= 0; // master 1 grant
            m2_grant <= 0; // master 2 grant
        end else begin
            if (m0_req ) begin
                m0_grant <= 1; // master 0 grant
                m1_grant <= 0; // master 1 grant
                m2_grant <= 0; // master 2 grant
            end else if (m1_req ) begin
                m0_grant <= 0; // master 0 grant
                m1_grant <= 1; // master 1 grant
                m2_grant <= 0; // master 2 grant
            end else if (m2_req) begin
                m0_grant <= 0; // master 0 grant
                m1_grant <= 0; // master 1 grant
                m2_grant <= 1; // master 2 grant
            end else begin
                m0_grant <= 1; // master 0 grant
                m1_grant <= 0; // master 1 grant
                m2_grant <= 0; // master 2 grant
            end
        end
    end

 
    always @(*) begin
        if (m0_grant) begin
            s_address = m0_address; // slave address
            s_wr = m0_wr; // slave write
            s_din = m0_dout; // slave data in
        end else if (m1_grant) begin
            s_address = m1_address; // slave address
            s_wr = m1_wr; // slave write
            s_din = m1_dout; // slave data in
        end else if (m2_grant) begin
            s_address = m2_address; // slave address
            s_wr = m2_wr; // slave write
            s_din = m2_dout; // slave data in
        end else begin
            s_address = 8'b0; // slave address
            s_wr = 1'b0; // slave write
            s_din = 32'b0; // slave data in
        end

        if (s_address >= 8'h00 && s_address <= 8'h1F) begin
            s_sel = 8'h01; // slave 0
        end else if (s_address >= 8'h20 && s_address <= 8'h3F) begin
            s_sel = 8'h02; // slave 1
        end else if (s_address >= 8'h40 && s_address <= 8'h5F) begin
            s_sel = 8'h04; // slave 2
        end else if (s_address >= 8'h60 && s_address <= 8'h7F) begin
            s_sel = 8'h08; // slave 3
        end else if (s_address >= 8'h80 && s_address <= 8'h9F) begin
            s_sel = 8'h10; // slave 4
        end else if (s_address >= 8'hA0 && s_address <= 8'hBF) begin
            s_sel = 8'h20; // slave 5
        end else if (s_address >= 8'hC0 && s_address <= 8'hDF) begin
            s_sel = 8'h40; // slave 6
        end else if (s_address >= 8'hE0 && s_address <= 8'hFF) begin
            s_sel = 8'h80; // slave 7
        end else begin
            s_sel = 8'h00; // 범위 밖
        end
    end

    always @(*) begin

        s0_dout = 32'b0; // slave 0 data out
        s1_dout = 32'b0; // slave 1 data out
        s2_dout = 32'b0; // slave 2 data out
        s3_dout = 32'b0; // slave 3 data out
        s4_dout = 32'b0; // slave 4 data out
        s5_dout = 32'b0; // slave 5 data out 
        s6_dout = 32'b0; // slave 6 data out
        s7_dout = 32'b0; // slave 7 data out

        if (!s_wr && s_sel != 8'h00) begin // when read operation
            case(s_sel)
                8'h01: s0_dout = mem[s_address]; // slave 0 data out
                8'h02: s1_dout = mem[s_address]; // slave 1 data out
                8'h04: s2_dout = mem[s_address]; // slave 2 data out
                8'h08: s3_dout = mem[s_address]; // slave 3 data out
                8'h10: s4_dout = mem[s_address]; // slave 4 data out
                8'h20: s5_dout = mem[s_address]; // slave 5 data out
                8'h40: s6_dout = mem[s_address]; // slave 6 data out
                8'h80: s7_dout = mem[s_address]; // slave 7 data out
                default: ;
            endcase
        end
    end

    // master data in
    always @(*) begin
        if (!s_wr) begin
            case (s_sel)
                8'h01: m_din = s0_dout; // master data in s0
                8'h02: m_din = s1_dout; // master data in s1
                8'h04: m_din = s2_dout; // master data in s2
                8'h08: m_din = s3_dout; // master data in s3
                8'h10: m_din = s4_dout; // master data in s4 
                8'h20: m_din = s5_dout; // master data in s5
                8'h40: m_din = s6_dout; // master data in s6
                8'h80: m_din = s7_dout; // master data in s7 
                default: m_din = 32'b0; // master data in 0
            endcase
        end else begin
            // write operation 
            m_din = 32'b0;  // master data in 0
        end
    end

endmodule

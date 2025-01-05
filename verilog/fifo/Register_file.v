module Register_file(
    input clk, // Clock signal
    input wr_en, // Write enable signal
    input [2:0] wr_addr, // Write address (3-bit)
    input [31:0] wr_data, // Data to write (32-bits)
    input [2:0] rd_addr, // Read address (3-bit)

    output reg [31:0] rdata // Data read from the register file (32-bits)
);

    // Define memory array (8 locations, each 32-bits wide)
    reg [31:0] mem [7:0];

    // Process block that occurs on each clock edge
    always @(posedge clk) begin
        // Writing data to memory if write enable is active
        if (wr_en) begin
            mem[wr_addr] <= wr_data; // Write data to the register file at wr_addr
        end
        // Reading data from memory if read enable is active
        rdata <= mem[rd_addr]; // Read data from the register file at rd_addr
    end
endmodule

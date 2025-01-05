`include "fifo_cal.v"
`include "fifo_ns.v"
`include "fifo_out.v"
`include "Register_file.v"

module fifo (
    input clk,  // Clock 
    input reset_n, // Active-low  => reset by 0
    input rd_en, // Read enable 
    input wr_en, // Write enable 
    input [31:0] d_in, // Data input

    output reg [31:0] d_out, // Data output
    output reg full, // Full flag
    output reg empty, // Empty flag 
    output reg wr_ack, // Write acknowledge
    output reg wr_err, // Write error
    output reg rd_ack, // Read acknowledge    
    output reg rd_err, // Read error
    output reg [3:0] data_count, // Data count vector
    output reg [3:0] next_data_count, // Next data count vector 

    output reg [2:0] state, state_next, // State and next state
    output reg [2:0] head, tail
);

    // Internal signals
    // reg [2:0] state, state_next; // State and next state
    // reg [2:0] head, next_head; // Head pointer and next head pointer
    // reg [2:0] tail, next_tail; // Tail pointer and next tail pointer
    reg [2:0] next_head; // Head pointer and next head pointer
    reg [2:0] next_tail; // Tail pointer and next tail pointer
    reg we, re; // Write and read enable
    reg [31:0] rdata; // Data read from the register file
    reg [31:0] wdata; // Data to write to the register file

    // Instantiate modules
    fifo_cal fifo_cal_inst (
        .state(state),
        .head(head),
        .tail(tail),
        .data_count(data_count),
        .we(we),
        .re(re),
        .next_head(next_head),
        .next_tail(next_tail),
        .next_data_count(next_data_count)
    );
    // Register file instantiation
    Register_file register_inst (
        .clk(clk),
        .wr_en(we),
        .wr_addr(tail),
        .wr_data(wdata),
        .rd_addr(head),
        .rdata(rdata)
    );

    // FIFO output and next state instantiation
    fifo_out fifo_out_inst (
        .state(state),
        .data_count(data_count),
        .wr_ack(wr_ack),
        .rd_ack(rd_ack),
        .wr_err(wr_err),
        .rd_err(rd_err),
        .empty(empty),
        .full(full)
    );

    // FIFO next state instantiation
    fifo_ns fifo_ns_inst (
        .wr_en(wr_en),
        .rd_en(rd_en),
        .state(state),
        .full(full),
        .empty(empty),
        .next_state(state_next)
    );

    // Write and read data assignment
    always @(*) begin
        wdata = d_in; // Write data to register file
    end

    // State, head, tail, and data count assignment
    always @(posedge clk or negedge reset_n) begin
        if (~reset_n) begin
            state <= 3'b000; // INIT state
            head <= 3'b000; // Reset head pointer
            tail <= 3'b000; // Reset tail pointer
            data_count <= 4'b0000; // Reset data count
            d_out <= 32'b0; // Reset data output
        end else begin
            state <= state_next; // Update state
            head <= next_head; // Update head pointer
            tail <= next_tail; // Update tail pointer
            data_count <= next_data_count; // Update data count
            if (re) begin
                d_out <= rdata; // Update data output on read
            end
        end
    end

endmodule

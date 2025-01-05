module fifo_out(
    input [2:0] state, // Current state
    input [3:0] data_count, // Current data count vector

    output reg wr_ack, // Write acknowledge
    output reg rd_ack, // Read acknowledge
    output reg wr_err, // Write error
    output reg rd_err, // Read error
    output reg empty, // Data empty signal
    output reg full // Data full signal
);

    always @(*) begin
        // Default values
            wr_ack = 0; // Write acknowledge
            rd_ack = 0; // Read acknowledge
            wr_err = 0; // Write error
            rd_err = 0; // Read error
            empty   = 0; // Queue is empty
            full    = 0; // Queue is full
        case(state)
            3'b000: begin // INIT
            end
            3'b001: begin // NO_OP
                if (data_count == 0) begin
                    empty = 1; // Queue is empty
                end else if (data_count == 8) begin
                    full = 1; // Queue is full
                end
            end
            3'b010: begin // WRITE
                wr_ack = 1; // Write acknowledge
                if (data_count == 8) begin
                    full = 1; // Queue is full
                end 
            end
            3'b011: begin // WR_ERROR
                wr_ack = 1; // Write acknowledge
                wr_err = 1; // Write error
                full = 1; // Queue is full
            end
            3'b100: begin // READ
                rd_ack = 1; // Read acknowledge
                if (data_count == 0) begin
                    empty = 1; // Queue is empty
                end
            end
            3'b101: begin // RD_ERROR
                rd_err = 1; // Read error
                empty = 1; // Queue is empty
            end
            default: begin
                wr_ack = 0; // Write acknowledge
                rd_ack = 0; // Read acknowledge
                wr_err = 0; // Write error
                rd_err = 0; // Read error
                empty = 0; // Queue is empty
                full = 0; // Queue is full
            end
        endcase
    end
endmodule

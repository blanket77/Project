module fifo_ns(
    input wr_en, // Write enable
    input rd_en, // Read enable
    input [2:0] state, // Current state
    input full, // Full flag
    input empty, // Empty flag
    
    output reg [2:0] next_state // Next state
);

    always @(*) begin
        case(state)
            3'b000: begin // INIT
                if (rd_en && empty) begin // If read enable and empty
                    next_state = 3'b101; // rd_en && emptyOR
                end else if (wr_en && full) begin // If write enable and full
                    next_state = 3'b011; // wr_en && fullOR
                end else begin
                    next_state = 3'b001; // NO_OP
                end
            end
            3'b001: begin // NO_OP
                if (wr_en && !full) begin  // If write enable and not full
                    next_state = 3'b010; // WRITE
                end else if (rd_en && !empty) begin
                    next_state = 3'b100; // READ
                end else if (wr_en && full) begin // If write enable and full
                    next_state = 3'b011; // wr_en && fullOR
                end else if (rd_en && empty) begin // If read enable and empty
                    next_state = 3'b101; // rd_en && emptyOR
                end else begin 
                    next_state = 3'b001; // NO_OP
                end
            end
            3'b010: begin // WRITE
                if (wr_en && !full) begin // If write enable and not full
                    next_state = 3'b010; // WRITE
                end else if (rd_en && !empty) begin // If read enable and not empty
                    next_state = 3'b100; // READ
                end else if (wr_en && full) begin // If write enable and full
                    next_state = 3'b011; // wr_en && fullOR
                end else begin
                    next_state = 3'b001; // NO_OP
                end
            end
            3'b011: begin // wr_en && fullOR
                if (rd_en && !empty) begin // If read enable and not empty
                    next_state = 3'b100; // READ
                end else if (wr_en && full) begin // If write enable and full
                    next_state = 3'b011; // wr_en && fullOR
                end else begin 
                    next_state = 3'b001; // NO_OP
                end
            end
            3'b100: begin // READ
                if (wr_en && !full) begin
                    next_state = 3'b010; // WRITE
                end else if (rd_en && !empty) begin // If read enable and not empty
                    next_state = 3'b100; // READ
                end else if (rd_en && empty) begin // If read enable and empty
                    next_state = 3'b101; // rd_en && emptyOR
                end else begin
                    next_state = 3'b001; // NO_OP
                end
            end
            3'b101: begin // rd_en && emptyOR
                if (wr_en && !full) begin // If write enable and not full
                    next_state = 3'b010; // WRITE
                end else if (rd_en && empty) begin // If read enable and empty
                    next_state = 3'b101; // rd_en && emptyOR
                end else begin
                    next_state = 3'b001; // NO_OP
                end
            end
            default: begin
                next_state = 3'b000; // INIT
            end
        endcase
    end
endmodule

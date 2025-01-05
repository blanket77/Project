module fifo_cal(
    input [2:0] state,          // Current state
    input [2:0] head,           // Current head pointer
    input [2:0] tail,           // Current tail pointer
    input [3:0] data_count,     // Current data count vector

    output reg we,              // Register file write enable
    output reg re,              // Register file read enable
    output reg [2:0] next_head, // Next head pointer
    output reg [2:0] next_tail, // Next tail pointer
    output reg [3:0] next_data_count // Next data count vector
);

    // Define maximum data count based on FIFO depth (e.g., 8)
    parameter MAX_DATA_COUNT = 8;

    always @(*) begin
        // Initialize outputs
        we = 0;
        re = 0;
        next_head = head;
        next_tail = tail;
        next_data_count = data_count;

        case(state)
            3'b000: begin // INIT
                next_head = 0;
                next_tail = 0;
                next_data_count = 0;
                we = 1; // Enable write
            end

            3'b001: begin // NO_OP
                // no operation: maintain current state
            end

            3'b010: begin // WRITE
                if (data_count < MAX_DATA_COUNT) begin
                    next_tail = (tail == 3'b111) ? 3'b000 : tail + 1; // Increment tail pointer
                    next_data_count = data_count + 1; // Increment data count
                    we = 1; // Enable write
                end else begin 
                    // data_count reaches MAX_DATA_COUNT: WR_ERROR state transition
                end
            end

            3'b011: begin // WR_ERROR
                // FIFO is full: writing disabled
                we = 0;
            end

            3'b100: begin // READ
                if (data_count > 0) begin
                    next_head = (head == 3'b111) ? 3'b000 : head + 1;
                    next_data_count = data_count - 1;
                    re = 1; // Enable read
                end else begin
                    // data_count reaches 0: RD_ERROR state transition
                end
            end

            3'b101: begin // RD_ERROR
                // fifo is empty: reading disabled
                re = 0;
            end

            default: begin
                next_head = head; // Maintain head pointer
                next_tail = tail; // Maintain tail pointer
                next_data_count = data_count; // Maintain data count
            end
        endcase
    end
endmodule

// //in the top-level module ///////////////////
`timescale 1ns/1ns

module testbench();

reg clk, reset;


//Initialize clocks and index
initial begin
    clk = 1'b0;
end

//Toggle the clocks
always begin
    #10
    clk  = !clk;
end

//Intialize and drive signals
initial begin
    reset  = 1'b0;
    #10 
    reset  = 1'b1;
    #30
    reset  = 1'b0;
end

wire signed [17:0] u_neighbor_0;
wire signed [17:0] u_neighbor_1;
wire signed [17:0] u_neighbor_2;
wire signed [17:0] u_neighbor_3;
wire signed [17:0] u_neighbor_4;
wire signed [17:0] u_neighbor_5;

assign u_neighbor_0 = 18'b00_0000000000010000;
assign u_neighbor_1 = 18'b00_0001000000000000;
assign u_neighbor_2 = 18'b00_0000001000000000;
assign u_neighbor_3 = 18'b00_0000010000000000;
assign u_neighbor_4 = 18'b00_0000010000000000;
assign u_neighbor_5 = 18'b00_1000000000000000;

wire signed [17:0] alpha;
assign alpha = 18'b01_0000000000000000;

wire signed [17:0] beta;
assign beta = 18'b00_0100000000000000;

wire is_frozen;
reg  is_frozen_reg [10:0]; // stores frozen vals for the column

reg  [15:0] write_addr_u_curr;
reg  [15:0] write_addr_v_curr;
reg  [15:0] read_addr_u_curr;
reg  [15:0] read_addr_v_curr;
reg         write_en_u_curr;
reg         write_en_v_curr;
reg  [17:0] write_data_u_curr;
reg  [17:0] write_data_v_curr;
wire [17:0] v_next;
wire [17:0] u_next;
wire [17:0] u_curr;
wire [17:0] v_curr;


/*
neighbor indexing: 
    even:
        4 0 5 
        2 x 3
        - 1 -
    odd:
        - 0 - 
        2 x 3
        4 1 5
*/

diffusion_solver solver_inst (
    .u_neighbor_0 (u_neighbor_0), // top
    .u_neighbor_1 (u_neighbor_1), // bottom
    .u_neighbor_2 (beta),         // left
    .u_neighbor_3 (beta),         // right
    .u_neighbor_4 (beta),         // left corner
    .u_neighbor_5 (beta),         // right corner
    .u_curr       (u_curr),
    .v_next       (v_next),
    .alpha        (alpha),
    .beta         (beta),
      
    .u_next       (u_next),
    .is_frozen    (is_frozen)
);

M10K_1000_8 M10k_u_curr ( 
    .q             (u_curr), 
    .d             (write_data_u_curr), 
    .write_address (write_addr_u_curr),
    .read_address  (read_addr_u_curr),
    .we            (write_en_u_curr),
    .clk           (clk)
);

M10K_1000_8 M10k_v_next ( // we only store v_next bc v_next = v_curr + gamma, but we never need to store v_curr
    .q             (v_next), 
    .d             (write_data_v_curr), 
    .write_address (write_addr_v_curr),
    .read_address  (read_addr_v_curr),
    .we            (write_en_v_curr),
    .clk           (clk)
);

// ------------------------- //
//       STATE MACHINE       //
// ------------------------- //

// 11 nodes

// reg [15:0] idx;

reg [4:0] state;

always @(posedge clk) begin
    if (reset) begin
        write_data_u_curr <= beta;
        write_data_v_curr <= 18'd0;
        write_addr_u_curr <= 16'd0;
        write_addr_v_curr <= 16'd0;
        write_en_u_curr <= 1'd1;
        write_en_v_curr <= 1'd1;

        read_addr_u_curr <= 16'd0;
        read_addr_v_curr <= 16'd0;

        state  <= 5'd0;
    end
    else begin
        case (state)

            5'd0: begin // initialization
                // SETTING INITIAL U AND V VALUES //
                // only center node is frozen, set to 1
                if (write_addr_u_curr == 16'd3) begin 
                    // on next cycle, it'll be the center node
                    write_data_u_curr; <= 18'd0;
                    write_data_v_curr <= 18'b01_0000000000000000 + gamma;
                end
                else if (write_addr_u_curr == 16'd2 || write_addr_u_curr == 16'd4) begin
                    // the nodes around the center node are receptive, but not frozen 
                    write_data_u_curr <= 18'd0;
                    write_data_v_curr <= beta + gamma;
                end
                else begin
                    // all other nodes are nonreceptive
                    write_data_u_curr <= beta;
                    write_data_v_curr <= 18'd0;
                end

                // MOVE FORWARD OR STOP //
                if (write_addr_u_curr >= 16'd10) begin
                    state <= 5'd1;
                    write_addr_u_curr <= 16'd0;
                    write_addr_v_curr <= 16'd0;
                    write_en_u_curr <= 1'd0;
                    write_en_v_curr <= 1'd0;
                end
                else begin
                    state <= 5'd0;
                    write_addr_u_curr <= write_addr_u_curr + 16'd1;
                    write_addr_v_curr <= write_addr_v_curr + 16'd1;
                    write_en_u_curr <= 1'd1;
                    write_en_v_curr <= 1'd1;
                end
            end
            5'd1: begin
                
                write_data_u_curr <= u_next;

            end
            5'd2: begin

            end
            5'd3: begin

            end
            5'd4: begin

            end
            5'd5: begin

            end

        endcase
    end
end
endmodule



/*
    this module calculates the diffusion equation (u) of one cell 
    it also outputs whether this cell is frozen at the end of calculating the diffusion
    u_next = u_curr + alpha / 2 * (u_avg - u_curr)
    u_avg = avg u over all neighbors
*/
module diffusion_solver (
    input  wire signed [17:0] u_neighbor_0,
    input  wire signed [17:0] u_neighbor_1,
    input  wire signed [17:0] u_neighbor_2,
    input  wire signed [17:0] u_neighbor_3,
    input  wire signed [17:0] u_neighbor_4,
    input  wire signed [17:0] u_neighbor_5,
    input  wire signed [17:0] u_curr,
    input  wire signed [17:0] v_next, // this is calculated outside of this module, when we calculate our current u and v

    input  wire signed [17:0] alpha,
    input  wire signed [17:0] beta,

    output wire signed [17:0] u_next,

    output wire        is_frozen
);

    wire signed [17:0] u_avg;
    wire signed [17:0] laplace_out;

    // s = u + v
    // frozen if s >= 1
    assign is_frozen = ((u_next + v_next) >= 18'b01_0000000000000000);
    assign u_next = u_curr + laplace_out;

    signed_mult u_avg_calc ( // divide by 6 (num neighbors) -- mult by 1/6
        .out(u_avg),
        .a  (u_neighbor_0+u_neighbor_1+u_neighbor_2+u_neighbor_3+u_neighbor_4+u_neighbor_5),
        .b  (18'b00_0010101010101010) // 1/6
    );

    signed_mult laplace_calc ( // alpha / 2 * (u_avg - cell.u)
        .out(laplace_out),
        .a  (alpha >> 1),
        .b  (u_avg - u_curr)
    );

endmodule


//////////////////////////////////////////////////
//// signed mult of 2.16 format 2'comp////////////
//////////////////////////////////////////////////

module signed_mult (out, a, b);
	output 	signed  [17:0]	out;
	input 	signed	[17:0] 	a;
	input 	signed	[17:0] 	b;
	// intermediate full bit length
	wire 	signed	[35:0]	mult_out;
	assign mult_out = a * b;
	// select bits for 7.20 fixed point
	assign out = {mult_out[35], mult_out[34:16]};
endmodule
//////////////////////////////////////////////////
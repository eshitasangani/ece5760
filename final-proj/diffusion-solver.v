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

wire [17:0] u_neighbors [0:5];

assign u_neighbors[0] = 18'b00_0000000000010000;
assign u_neighbors[1] = 18'b00_0001000000000000;
assign u_neighbors[2] = 18'b00_0000001000000000;
assign u_neighbors[3] = 18'b00_0000010000000000;
assign u_neighbors[4] = 18'b00_0000010000000000;
assign u_neighbors[5] = 18'b00_1000000000000000;

diffusion_solver solver_inst (
    .u_neighbors(u_neighbors),
    .u_curr     (beta),
    .v_next     (18'd0),
    .alpha      (alpha),
    .beta       (beta),
    
    .u_next     (u_next),
    .is_frozen  (is_frozen)
)

endmodule

/*
neighbor indexing: 
        - 0 1 
        2 x 3
        4 5 -
*/

/*
    this module calculates the diffusion equation (u) of one cell 
    it also outputs whether this cell is frozen at the end of calculating the diffusion
    u_next = u_curr + alpha / 2 * (u_avg - u_curr)
    u_avg = avg u over all neighbors
*/
module diffusion_solver (
    input  wire [17:0] u_neighbors [0:5],
    input  wire [17:0] u_curr,
    input  wire [17:0] v_next, // this is calculated outside of this module, when we calculate our current u and v

    input  wire [17:0] alpha,
    input  wire [17:0] beta,

    output wire [17:0] u_next,

    output wire        is_frozen
);

    wire [17:0] u_avg;
    wire [17:0] laplace_out;

    // s = u + v
    // frozen if s >= 1
    assign is_frozen = ((u_next + v_next) >= 18'b01_0000000000000000);

    signed_mult u_avg_calc ( // divide by 6 (num neighbors) -- mult by 1/6
        .out(u_avg),
        .a  (u_neighbors[0]+u_neighbors[1]+u_neighbors[2]+u_neighbors[3]+u_neighbors[4]+u_neighbors[5]),
        .b  (18'b00_0010101010101010)
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
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

wire done_outside;
wire done_inside;

complex_iterator complex_iterator_inst (
    .clk          (clk),
    .reset        (reset),
    .max_iter     (16'd1000),

    .c_r          (27'b1110_00000000000000000000000),
    .c_i          (27'b0000_00000000000000000000000),

    .done_outside (done_outside),
    .done_inside  (done_inside)
);


endmodule


module complex_iterator (
    input  wire               clk,
    input  wire               reset,
    input  wire        [15:0] max_iter,

    input  wire signed [26:0] c_r,
    input  wire signed [26:0] c_i,

    output wire               done_outside,
    output wire               done_inside
);
    
    reg  signed [26:0] z_r;
    reg  signed [26:0] z_i;
    reg  signed [26:0] z_i_sq;
    reg  signed [26:0] z_r_sq;
    reg  signed [15:0] iter;
    wire signed [26:0] z_out;

    // top branch internal sigs

    wire signed [26:0] z_r_i_sq_sum;
    wire signed [26:0] z_r_tmp;
    wire signed [26:0] z_r_sq_tmp;

    // bottom branch internal sigs
    wire signed [26:0] z_r_i_mul;
    wire signed [26:0] z_i_tmp;
    wire signed [26:0] z_i_sq_tmp;


    always @(posedge clk) begin
        if (reset) begin
            z_r    <= 27'd0;
            z_i    <= 27'd0;
            z_r_sq <= 27'd0;
            z_i_sq <= 27'd0;
            iter   <= 16'd0;
        end
        else begin
            z_r    <= z_r_tmp;
            z_i    <= z_i_tmp;
            z_r_sq <= z_r_sq_tmp;
            z_i_sq <= z_i_sq_tmp;
            iter   <= iter + 1'b1;
        end
    end

    // if sqr magnitude (z_out) is greater than 4 (2^2):
    //      write outside color to vga
    assign done_outside = z_out > 27'b0100_00000000000000000000000;

    // if we exceed max iterations:
    //      write inside color to vga
    assign done_inside  = iter  > max_iter;
    
    ////////////////////////////////////////////////////
    /////////////////// TOP BRANCH /////////////////////
    ////////////////////////////////////////////////////

    assign z_r_tmp = z_r_sq - z_i_sq + c_r;
    
    signed_mult z_r_n_1_sq (
        .out (z_r_sq_tmp),
        .a   (z_r_tmp),
        .b   (z_r_tmp)
    );

    ////////////////////////////////////////////////////
    ///////////////// BOTTOM BRANCH ////////////////////
    ////////////////////////////////////////////////////

    signed_mult z_complex_mult (
        .out (z_r_i_mul),
        .a   (z_r),
        .b   (z_i)
    );

    assign z_i_tmp = (z_r_i_mul << 1) + c_i;

    signed_mult z_i_n_1_sq (
        .out (z_i_sq_tmp),
        .a   (z_i_tmp),
        .b   (z_i_tmp)
    );


    assign z_out = z_i_sq_tmp + z_r_sq_tmp;


endmodule








//////////////////////////////////////////////////
//// signed mult of 4.23 format 2'comp////////////
//////////////////////////////////////////////////

module signed_mult (out, a, b);
	output 	signed  [26:0]	out;
	input 	signed	[26:0] 	a;
	input 	signed	[26:0] 	b;
	// intermediate full bit length
	wire 	signed	[53:0]	mult_out;
	assign mult_out = a * b;
	// select bits for 7.20 fixed point
	assign out = {mult_out[53], mult_out[48:23]};
endmodule
//////////////////////////////////////////////////

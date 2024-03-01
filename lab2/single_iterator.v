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

wire [15:0] iter;
wire        done;

complex_iterator complex_iterator_inst (
    .clk          (clk),
    .reset        (reset),
    .max_iter     (16'd1000),

    .c_r          ( 27'b0001_00000000000000000000000),
    .c_i          ( 27'b1111_00000000000000000000000),

    .iter         (iter),
    .done         (done)
);

//always begin
//    if (done) begin
//	$finish;
//    end
//end

endmodule


module complex_iterator (
    input  wire               clk,
    input  wire               reset,
    input  wire        [15:0] max_iter,

    input  wire signed [26:0] c_r,
    input  wire signed [26:0] c_i,

    output wire        [15:0] iter,
    output wire               done
);
    
    wire signed [26:0] z_r;
    wire signed [26:0] z_i;
    wire signed [26:0] z_i_sq;
    wire signed [26:0] z_r_sq;

    wire signed [26:0] z_out;

    wire signed [26:0] z_r_i_sq_sum;
    wire signed [26:0] z_r_tmp;
    wire signed [26:0] z_r_sq_tmp;

    // bottom branch internal sigs
    wire signed [26:0] z_r_i_mul;
    wire signed [26:0] z_i_tmp;
    wire signed [26:0] z_i_sq_tmp;

    // testing 
    wire intermediate_done;
    wire intermediate_done_r;
    wire intermediate_done_i;

    ////////////////////////////////////////////////////
    /////////////////// ENABLE REGISTERS ///////////////
    ////////////////////////////////////////////////////

    enable_reg #(27) z_r_reg(
	    .clk (clk), 
	    .rst (reset), 
	    .en  (~done), 
        .d   (z_r_tmp), 
        .q   (z_r)
    );

    enable_reg #(27) z_i_reg(
	    .clk (clk), 
	    .rst (reset), 
	    .en  (~done), 
        .d   (z_i_tmp), 
        .q   (z_i)
    );

    enable_reg #(27) z_r_sq_reg(
	    .clk (clk), 
	    .rst (reset), 
	    .en  (~done), 
        .d   (z_r_sq_tmp), 
        .q   (z_r_sq)
    );

    enable_reg #(27) z_i_sq_reg(
	    .clk (clk), 
	    .rst (reset), 
	    .en  (~done), 
        .d   (z_i_sq_tmp), 
        .q   (z_i_sq)
    );

    enable_reg #(16) counter_reg(
	    .clk (clk), 
	    .rst (reset), 
	    .en  (~done), 
        .d   (iter + 1'b1), 
        .q   (iter)
    );
    
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

    assign z_i_tmp = (z_r_i_mul <<< 1) + c_i;

    signed_mult z_i_n_1_sq (
        .out (z_i_sq_tmp),
        .a   (z_i_tmp),
        .b   (z_i_tmp)
    );


    assign intermediate_done_r = ((z_i)  > 27'sb0010_00000000000000000000000) || ( (z_i) < -27'sb0010_00000000000000000000000);
    assign intermediate_done_i = ( (z_r) > 27'sb0010_00000000000000000000000) || ( (z_r) < -27'sb0010_00000000000000000000000);

    assign intermediate_done = (intermediate_done_i | intermediate_done_r);

	assign z_out = z_i_sq_tmp + z_r_sq_tmp;

    assign done = (intermediate_done) || (iter == max_iter);
//assign done = (intermediate_done) | (z_out > 27'sb0100_00000000000000000000000) | (iter > max_iter);

	// assign done = ( z_out > 27'b0100_00000000000000000000000) | (iter > max_iter);


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


//////////////////////////////////////////////////
/////////////// enable register //////////////////
//////////////////////////////////////////////////

module enable_reg #(parameter BITWIDTH = 27) (
	input  wire clk, 
	input  wire rst, 
	input  wire en, 
	input  wire signed [BITWIDTH-1:0] d, 
	output wire signed [BITWIDTH-1:0] q);

    reg signed [BITWIDTH-1:0] d_reg;

    always @(posedge clk) begin
        if (rst) begin
            d_reg <= 27'd0;
        end
        else if (en) begin
            d_reg <= d;
        end
        else begin
            d_reg <= q;
        end
    end

    assign q = d_reg;

endmodule 



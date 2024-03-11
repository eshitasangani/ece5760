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

wire [17:0] u_next;

drum_syn drum_syn_inst (
    .clk          (clk),
    .reset        (reset),
    .rho          (18'b0_00010000000000000), // 0.0625
    .u_top        (18'd0),
    .u_bottom     (18'd0),
    .u_left       (18'd0),
    .u_right      (18'd0),
    .u_prev_i     (18'b0_00100000000000000),
    .u_curr_i     (18'b0_00100000000000000),
    .u_next       (u_next)
);

endmodule

//////////////////////////////////////////////////
//////// one node of drum synthesizer ////////////
//////////////////////////////////////////////////

module drum_syn (
    input wire                clk,
    input wire                reset,

    input wire         [17:0] rho,

    input wire signed  [17:0] u_top,
    input wire signed  [17:0] u_bottom,
    input wire signed  [17:0] u_left,
    input wire signed  [17:0] u_right,
    input wire signed  [17:0] u_prev_i,
    input wire signed  [17:0] u_curr_i,
    
    output wire signed [17:0] u_next
);

    reg signed [17:0] u_prev;
    reg signed [17:0] u_curr;

    // intermediate internal signals

    wire signed [17:0] int_sum;
    wire signed [17:0] rho_mult;
    wire signed [17:0] int_mid;

    assign int_sum = u_left + u_right + u_bottom + u_top - (u_curr << 2);
    assign int_mid = (rho_mult + (u_curr << 1) - u_prev + (u_prev >>> 9));
    assign u_next = int_mid - (int_mid >>> 9);

    signed_mult rho_mult_inst (
        .out (rho_mult),
        .a   (rho),
        .b   (int_sum)
    );

    always @(posedge clk) begin
        if (reset) begin
            u_prev <= u_prev_i;
            u_curr <= u_curr_i;
        end
        else begin
            u_prev <= u_curr;
            u_curr <= u_next;
        end
    end


endmodule


//////////////////////////////////////////////////
//// signed mult of 1.17 format 2'comp ///////////
//////////////////////////////////////////////////

module signed_mult (out, a, b);
	output 	signed  [17:0]	out;
	input 	signed	[17:0] 	a;
	input 	signed	[17:0] 	b;
	// intermediate full bit length
	wire 	signed	[35:0]	mult_out;
	assign mult_out = a * b;
	// select bits for 1.17 fixed point
	assign out = {mult_out[35], mult_out[33:17]};
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
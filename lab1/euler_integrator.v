// //in the top-level module ///////////////////
`timescale 1ns/1ns

module testbench();

// // clock divider to slow system down for testing
// reg [4:0] count;
// // analog update divided clock
// always @ (posedge CLOCK_50) 
// begin
//         count <= count + 1; 
// end	
// assign AnalogClock = (count==0);		

reg clk_50, reset;

wire signed [26:0]  x_o;
wire signed [26:0]  y_o;
wire signed [26:0]  z_o;

wire signed [26:0] sigma;
wire signed [26:0] beta;
wire signed [26:0] rho;
wire signed [26:0] x_i;
wire signed [26:0] y_i;
wire signed [26:0] z_i;

// INITIAL VALUES //
assign x_i = -27'b0000001_00000000000000000000; //-1
assign y_i = 27'b0000000_00011001100110011001; //0.1
assign z_i = 27'b0011001_00000000000000000000; //25
assign sigma = 27'b0001010_00000000000000000000; //10
assign beta = 27'b0000010_10101010101010101010; //2.66666
assign rho = 27'b0011100_00000000000000000000; //28

//Initialize clocks and index
initial begin
    clk_50 = 1'b0;
end

//Toggle the clocks
always begin
    #10
    clk_50  = !clk_50;
end

//Intialize and drive signals
initial begin
    reset  = 1'b0;
    #10 
    reset  = 1'b1;
    #30
    reset  = 1'b0;
end

euler_integrator DUT (
    .clk(clk_50),
    .reset(reset),
    .sigma(sigma),
    .beta(beta),
    .rho(rho),
    .x_i(x_i),
    .y_i(y_i),
    .z_i(z_i),
    .x_o(x_o),
    .y_o(y_o),
    .z_o(z_o)
);

endmodule

module euler_integrator(
    input                clk,
    input                reset,

    input  signed [26:0] sigma,
    input  signed [26:0] beta,
    input  signed [26:0] rho,
    input  signed [26:0] x_i,
    input  signed [26:0] y_i,
    input  signed [26:0] z_i,

    output signed [26:0] x_o,
    output signed [26:0] y_o,
    output signed [26:0] z_o
);
    // INTERNAL SIGNALS //
    wire signed [26:0] x;
    wire signed [26:0] y;
    wire signed [26:0] z;
    wire signed [4:0]  dt;

    assign dt = 5'd8;

    /////////////////////////////////////////////////
    //// dx /////////////////////////////////////////
    /////////////////////////////////////////////////
    wire signed [26:0] y_sub_x;
    wire signed [26:0] x_dt_shift;
    wire signed [26:0] dx_dt;

    assign y_sub_x = y - x;
    assign x_dt_shift = y_sub_x >>> dt;

    signed_mult sigma_x_mul (
        .out(dx_dt),
        .a(sigma),
        .b(x_dt_shift)
    );

    integrator x_integrator (
        .out(x_o),
        .funct(dx_dt),
        .InitialOut(x_i),
        .clk(clk),
        .reset(!reset)
    );

    assign x = x_o;


    /////////////////////////////////////////////////
    //// dy /////////////////////////////////////////
    /////////////////////////////////////////////////
    wire signed [26:0] rho_sub_z;
    wire signed [26:0] x_mul_sub;
    wire signed [26:0] y_shift_dt0;
    wire signed [26:0] y_shift_dt1;
    wire signed [26:0] dy_dt;

    assign rho_sub_z = rho - z;
    assign y_shift_dt0 = x >>> dt;
    assign y_shift_dt1 = y >>> dt;

    signed_mult x_mul_sub_mul (
        .out(x_mul_sub),
        .a(rho_sub_z),
        .b(y_shift_dt0)
    );

    assign dy_dt = x_mul_sub - y_shift_dt1;

    integrator y_integrator (
        .out(y_o),
        .funct(dy_dt),
        .InitialOut(y_i),
        .clk(clk),
        .reset(!reset)
    );

    assign y = y_o;

    /////////////////////////////////////////////////
    //// dz /////////////////////////////////////////
    /////////////////////////////////////////////////
    wire signed [26:0] x_y;
    wire signed [26:0] beta_z;
    wire signed [26:0] dz_dt;
    wire signed [26:0] z_shift_dt;

    assign z_shift_dt = z >>> dt;

    signed_mult x_y_mul (
        .out(x_y),
        .a(y_shift_dt0),
        .b(y)
    );

    signed_mult beta_z_mul (
        .out(beta_z),
        .a(beta),
        .b(z_shift_dt)
    );

    assign dz_dt = x_y - beta_z;

    integrator z_integrator (
        .out(z_o),
        .funct(dz_dt),
        .InitialOut(z_i),
        .clk(clk),
        .reset(!reset)
    );

    assign z = z_o;

endmodule


/////////////////////////////////////////////////
//// integrator /////////////////////////////////
/////////////////////////////////////////////////

module integrator(out,funct,InitialOut,clk,reset);
	output signed [26:0] out; 		//the state variable V
	input signed [26:0] funct;      //the dV/dt function
	input clk, reset;
	input signed [26:0] InitialOut;  //the initial state variable V
	
	wire signed	[26:0] out, v1new ;
	reg signed	[26:0] v1 ;
	
	always @ (posedge clk) 
	begin
		if (reset==0) //reset	
			v1 <= InitialOut ; // 
		else 
			v1 <= v1new ;	
	end
	assign v1new = v1 + funct ;
	assign out = v1 ;
endmodule

//////////////////////////////////////////////////
//// signed mult of 7.20 format 2'comp////////////
//////////////////////////////////////////////////

module signed_mult (out, a, b);
	output 	signed  [26:0]	out;
	input 	signed	[26:0] 	a;
	input 	signed	[26:0] 	b;
	// intermediate full bit length
	wire 	signed	[53:0]	mult_out;
	assign mult_out = a * b;
	// select bits for 7.20 fixed point
	assign out = {mult_out[53], mult_out[45:20]};
endmodule
//////////////////////////////////////////////////

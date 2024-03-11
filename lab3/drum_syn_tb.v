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

wire [17:0] u_next [29:0];
reg  [17:0] u_prev [29:0];
reg  [17:0] u_curr [29:0];

// BOTTOM IDX = 0
// TOP IDX = 29 (N-1)

generate
    genvar i;
    for ( i = 0; i < 30; i = i + 1 ) begin: gen1
        drum_syn drum_syn_inst (
            .rho          (18'b0_00010000000000000), // 0.0625
            .u_top        ((i == 29)  ? (18'd0) : u_curr[i+1]), // if we're at the top, 0 boundary condition
            .u_bottom     ((i == 0) ? (18'd0) : u_curr[i-1]), // if we're at the bottom, 0 boundary condition
            .u_left       (18'd0),
            .u_right      (18'd0),
            .u_prev       (u_prev[i]),
            .u_curr       (u_curr[i]),
            .u_next       (u_next[i])
        ); 
    end
endgenerate

// state machine
reg [4:0] state;
reg [5:0] idx; // this tracks where we are in the column

always @(posedge clk) begin

    if (reset) begin
        state <= 5'd0;
        idx   <= 6'd0;
        u_prev[0] <= 18'b0_00000000000000000;
        u_curr[0] <= 18'b0_00000000000000000;

        

        u_prev[15] <= 18'b0_00100000000000000; // 1/8
        u_curr[15] <= 18'b0_00100000000000000; // 1/8

        u_prev[29] <= 18'b0_00000000000000000;
        u_curr[29] <= 18'b0_00000000000000000;
    end
    else begin
        case (state)

            5'd0: begin // 

            end
            5'd1:
            5'd2:
            5'd3:


        endcase
    end

end


M10K_1000_8 M10k_u_prev ( 
    .q             (), 
    .d             (u_curr),
    .write_address (),
    .read_address  (),
    .we            (),
    .clk           (clk)
);

M10K_1000_8 M10k_u_curr ( 
    .q             (), 
    .d             (u_next),
    .write_address (),
    .read_address  (),
    .we            (),
    .clk           (clk)
);


endmodule



//////////////////////////////////////////////////
//////// one node of drum synthesizer ////////////
//////////////////////////////////////////////////

module drum_syn (
    input wire         [17:0] rho,

    input wire signed  [17:0] u_top,
    input wire signed  [17:0] u_bottom,
    input wire signed  [17:0] u_left,
    input wire signed  [17:0] u_right,
    input wire signed  [17:0] u_prev,
    input wire signed  [17:0] u_curr,
    
    output wire signed [17:0] u_next
);

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


//============================================================
// M10K module for testing
//============================================================
// See example 12-16 in 
// http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HDL_style_qts_qii51007.pdf
//============================================================

module M10K_1000_8( 
    output reg [17:0] q,
    input [17:0] d,
    input [18:0] write_address, read_address,
    input we, clk
);
	 // force M10K ram style
	 // 307200 words of 8 bits
    reg [17:0] mem [68266:0]  /* synthesis ramstyle = "no_rw_check, M10K" */;
	// reg [7:0] mem [153600:0]; // 2 solvers
	// reg [7:0] mem [76800:0]; // 4 solvers
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
        q <= mem[read_address]; // q doesn't get d in this clock cycle
    end
endmodule
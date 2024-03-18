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

// pio ports for initial values 
// TODO: make sure to set these to wires when we instantiate the pio ports!!
reg [17:0] pio_init;
reg        pio_init_done;


// this simulates our pio ports
reg [5:0] idx; // this tracks where we are in the column

always @(posedge clk) begin
    if (reset) begin
        idx <= 6'b0;
        pio_init <= 18'b0_00000000000100000;
    end
    else if (pio_init_done) begin

        if (idx <= 14) begin
            pio_init <= pio_init + 18'b0_00000000000001000;
            idx <= idx + 6'd1;
        end
        else if (idx == 15) begin
            pio_init <= pio_init;
            idx <= idx + 6'd1;
        end
        else if (idx > 15 && idx < 30) begin
            pio_init <= pio_init - 18'b0_00000000000001000;
            idx <= idx + 6'd1;
        end
    end
end


// BOTTOM IDX = 0
// TOP IDX = 29 (N-1)

// generate
//     genvar i;
//     for ( i = 0; i < 30; i = i + 1 ) begin: gen1
//         drum_syn drum_syn_inst (
//             .rho          (18'b0_00010000000000000), // 0.0625
//             .u_top        ((i == 29)  ? (18'd0) : u_curr[i+1]), // if we're at the top, 0 boundary condition
//             .u_bottom     ((i == 0) ? (18'd0) : u_curr[i-1]), // if we're at the bottom, 0 boundary condition
//             .u_left       (18'd0),
//             .u_right      (18'd0),
//             .u_prev       (u_prev[i]),
//             .u_curr       (u_curr[i]),
//             .u_next       (u_next[i])
//         ); 
//     end
// endgenerate

// from m10k blocks
wire [17:0] u_top;
wire [17:0] u_prev;

// register these and update every iteration
reg  [17:0] u_bottom;
reg  [17:0] u_curr;

// our output
wire [17:0] u_next;

// state machine
reg [4:0] state;

reg write_curr_en; // write to the n m10k block
reg [17:0] write_curr_data;
reg [18:0] write_curr_address;
reg [18:0] read_curr_address;

reg write_prev_en; // write to the n-1 m10k block
reg [17:0] write_prev_data;
reg [18:0] write_prev_address;
reg [18:0] read_prev_address;

reg top_flag;

M10K_1000_8 M10k_u_prev ( 
    .q             (u_prev), // we always read out u_prev from this m10k
    .d             (write_prev_data), 
    .write_address (write_prev_address),
    .read_address  (read_prev_address),
    .we            (write_prev_en),
    .clk           (clk)
);

M10K_1000_8 M10k_u_curr ( 
    .q             (u_top),  // we always read out u_top from this m10k
    .d             (write_curr_data), // we always write u_next to this m10k
    .write_address (write_curr_address),
    .read_address  (read_curr_address),
    .we            (write_curr_en),
    .clk           (clk)
);

drum_syn drum_syn_inst (
            .rho          (18'b0_00010000000000000), // 0.0625
            .u_top        ((top_flag) ? (18'd0) : u_top), // if we're at the top, 0 boundary condition
            .u_bottom     (u_bottom), // if we're at the bottom, 0 boundary condition (set in state machine reset)
            .u_left       (18'd0),
            .u_right      (18'd0),
            .u_prev       (u_prev),
            .u_curr       (u_curr),
            .u_next       (u_next)
); 

always @(posedge clk) begin

    if (reset) begin
        state <= 5'd0;
        idx   <= 6'd0;
        write_prev_address <= 19'd0;
        write_curr_address <= 19'd0;

        write_curr_en <= 1'b1;
        write_prev_en <= 1'b1;

        write_curr_data <= pio_init;
        write_prev_data <= pio_init;

        // start at bottom 
        read_prev_address <= 19'd0; 
        read_curr_address <= 19'd0;

        // set bottom boundary condition 
        u_bottom <= 18'd0; 
        
        // set u_curr
        
        u_curr <= 18'b0_00000000000100000;
        top_flag <= 1'b0;
    end
    else begin
        case (state)
            // FIRST TWO STATES ARE OUR INIT STATE MACHINE // 
            5'd0: begin // start writing initial values

                if (write_prev_address >= 19'd29) begin // once we are done writing all rows start processing

                    write_curr_en <= 1'b0;
                    write_prev_en <= 1'b0;

                    write_prev_address <= 19'd0;
                    write_curr_address <= 19'd0;

                    state <= 5'd2;
                end

                else begin 
                    write_prev_address <= write_prev_address + 19'd1;
                    write_curr_address <= write_curr_address + 19'd1;

                    write_prev_en <= 1'b1;
                    write_curr_en <= 1'b1;

                    write_prev_data <= pio_init;
                    write_curr_data <= pio_init;

                    pio_init_done <= 1'b1; 
                    state <= 5'd1;
                end
                
            end
            5'd1: begin // second init state
                pio_init_done <= 1'b0;
                state <= 5'd0;
            end


            // START OF OUR REGULAR STATE MACHINE // 
            5'd2: begin
                write_curr_en   <= 1'b0;
                write_prev_en   <= 1'b0;
                state <= 5'd3;
            end

            5'd3: begin // when we loop back to the bottom, we have to read u_curr and u_top from m10k
                u_curr <= u_top;
                read_curr_address <= read_curr_address + 19'd1;
                state <= 5'd4;
            end


            5'd4: begin // wait one cycle for read data to come back
                write_curr_en   <= 1'b0;
                write_prev_en   <= 1'b0;
                state <= 5'd5;
            end

            5'd5: begin
                // write u_next to n m10k (update NEXT timestep for this node)
                write_curr_en   <= 1'b1;
                write_curr_data <= u_next;

                // write u_curr to n-1 m10k (update PREV timestep for this node)
                write_prev_en   <= 1'b1;  
                write_prev_data <= u_curr;

                // move up one node (in CURR timestep)
                u_curr          <= u_top;  
                u_bottom        <= u_curr; 

                state <= 5'd6;
            end

            5'd6: begin
                write_curr_en   <= 1'b0;
                write_prev_en   <= 1'b0;
                
                // if (read_prev_address == 19'd28) begin
                //     top_flag <= 1'b1;
                // end
                
                // state transition 
                if (top_flag) begin // we've finished the column

                    // restart from the bottom!

                    u_bottom <= 18'd0; // set bottom boundary condition

                    top_flag <= 1'b0;

                    write_curr_address <= 19'd0;
                    write_prev_address <= 19'd0;

                    read_prev_address <= 19'd0;
                    

                    state <= 5'd3;
                end
                else begin
                    // move up one node (in CURR timestep)
                    
                    write_prev_address <= write_prev_address + 19'd1;
                    write_curr_address <= write_curr_address + 19'd1;

                    read_prev_address <= read_prev_address + 19'd1;
                    read_curr_address <= read_curr_address + 19'd1;

                    if (read_prev_address == 19'd28) begin
                        top_flag <= 1'b1;
                        read_curr_address <= 19'd0;
                    end

                    state <= 5'd4;
                end
                
            end

        endcase
    end

end




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
    wire signed [17:0] int_dif_left;
    wire signed [17:0] int_dif_right;
    wire signed [17:0] int_dif_bottom;
    wire signed [17:0] int_dif_top;

    wire signed [17:0] rho_mult;
    wire signed [17:0] int_mid;

    assign int_dif_left = u_left - u_curr;
    assign int_dif_right = u_right - u_curr;
    assign int_dif_bottom = u_bottom - u_curr;
    assign int_dif_top = u_top - u_curr;

    assign int_sum = int_dif_left + int_dif_right + int_dif_bottom + int_dif_top;
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
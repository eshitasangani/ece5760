// //in the top-level module ///////////////////
`timescale 1ns/1ns

module testbench();

reg clk, slow_clk, reset;
reg [31:0] timer;

//Initialize clocks and index
initial begin
    clk = 1'b0;
    slow_clk = 1'b0;
    timer = 32'd0;
end

//Toggle the clocks
always begin
    #2000
    clk  = !clk;
end

reg audio_request; 
always begin
    #18833
    audio_request <= 1'b1;
    #2000
    audio_request <= 1'b0;
end

//Intialize and drive signals
initial begin
    reset  = 1'b0;
    #10 
    reset  = 1'b1;
    #41666
    reset  = 1'b0;
end

// pio ports for initial values 
// TODO: make sure to set these to wires when we instantiate the pio ports!!
reg [17:0] pio_init;
reg        pio_init_done;


// this simulates our pio ports
reg [5:0] idx; // this tracks where we are in the column

// nonlinear rho calculation 
// reg [17:0] rho_eff;
// wire [17:0] rho_eff;

// wire [17:0] rho_0;
// wire [17:0] max_rho;
// wire [17:0] rho_gtension;

// always @(posedge clk) begin
//     timer <= timer + 32'd1;
//     if (reset) begin
//         idx <= 6'b0;
//         pio_init <= 18'b0_00000000000000000;
//         rho_eff <= 18'b0_01000000000000000;
//     end
//     else if (pio_init_done) begin

//         if (idx <= 14) begin
//             pio_init <= pio_init + 18'b0_00000010000000000;
//             idx <= idx + 6'd1;
//         end
//         else if (idx == 15) begin
//             pio_init <= pio_init;
//             idx <= idx + 6'd1;
//         end
//         else if (idx > 15 && idx < 30) begin
//             pio_init <= pio_init - 18'b0_00000010000000000;
//             idx <= idx + 6'd1;
//         end
//     end
// end

// reg audio_request [29:0]; // we only use the [15], but use an array so we can reset in the generate block 

// reg audio_request_ack [29:0]; // we only use the [15], but use an array so we can reset in the generate block 

// // THIS SIMULATES OUR AUDIO BUS MASTER STATE MACHINE REQUEST
// reg counter;
// always @(posedge slow_clk) begin
    
//     if (reset) begin
//         audio_request <= 1'b0;
//         counter <= 1'd0;
//     end
//     else if (counter == 1'd1) begin
//         audio_request <= 1'b1;
//         counter <= 1'd0;
//     end
//     else begin
//         audio_request <= audio_request_ack[15];
//         counter <= counter + 1'd1;
//     end
// end

// BOTTOM IDX = 0
// TOP IDX = 29 (N-1)


//=======================================================
//  DRUM SYNTH
//=======================================================

// pll fast clock 
// M10k memory clock
// wire 					pll_fast_clk_100 ;
// wire 					pll_fast_clk_100_locked ;

// pio ports for initial values 
// TODO: make sure to set these to wires when we instantiate the pio ports!!
// wire [31:0] pio_init;
// reg  [29:0] pio_init_done;
// wire        pio_init_val;



// BOTTOM IDX = 0
// TOP IDX = 29 (N-1)

// register this and update every iteration
// this has to be outside our generate so we can connect between columns
reg [17:0] u_curr [169:0];
reg [17:0] u_center [169:0];

wire [17:0] pio_step_x;
wire [17:0] pio_step_y;
wire [18:0] num_rows;

assign num_rows = 19'd170;
assign pio_step_x = 18'b0_00000000011000001;
assign pio_step_y = 18'b0_00000000011000001;
wire [18:0] half_num_rows;
assign half_num_rows = num_rows >>> 1;


//=======================================================
//  NONLINEAR RHO
//=======================================================
// nonlinear rho calculation 
reg  [17:0] rho_eff;
wire [17:0] rho_0;
wire [17:0] max_rho;
wire [17:0] rho_gtension;

assign rho_0 = 18'b0_01000000000000000;
assign max_rho = 18'b0_01111110000000000;

signed_mult nonlinear_rho (
        .out (rho_gtension),
        .a   (u_center[85]>>>4),
        .b   (u_center[85]>>>4)
    );

//=======================================================
//  DRUM SYNTH STATE MACHINE
//=======================================================

// reg [29:0] idx;

always @(posedge clk) begin
	if (reset) begin
		// rho
		rho_eff <= 18'b0_01000000000000000;
	end
	else begin
		// if (idx[15] == 19'd15) begin // output the center node!
		// 	u_center <= u_curr[15];
		// end

		if (max_rho >= (rho_0 + rho_gtension)) begin
        	rho_eff <= rho_0 + rho_gtension;
            //rho_eff <= max_rho;
		end
		else begin
			rho_eff <= max_rho;
            //rho_eff <= rho_0 + rho_gtension;
		end
	end
end



generate
    genvar i;
    for ( i = 0; i < 170; i = i + 1 ) begin: gen1

        // from m10k blocks
        wire [17:0] u_top ;
        wire [17:0] u_prev;

        // our output
        wire [17:0] u_next;

        // register this and update every iteration
        reg  [17:0] u_bottom;

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

		reg [18:0] addr_idx; 

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
                    .rho          (rho_eff), // 0.0625 = 18'b0_00010000000000000
                    .u_top        ((top_flag) ? (18'd0) : u_top), // if we're at the top, 0 boundary condition
                    .u_bottom     (u_bottom), // if we're at the bottom, 0 boundary condition (set in state machine reset)
                    .u_left       ((i == 0) ? (18'd0) : u_curr[i-1]),
                    .u_right      ((i == 169) ? (18'd0) : u_curr[i+1]),
                    .u_prev       (u_prev),
                    .u_curr       (u_curr[i]),
                    .u_next       (u_next)
        ); 

        always @(posedge clk) begin
			// audio_request_ack[i] <= audio_request;

            if (reset) begin
                state <= 5'd0;
                write_prev_address <= 19'd0;
                write_curr_address <= 19'd0;
                addr_idx <= 19'd0;

                write_curr_en <= 1'b1;
                write_prev_en <= 1'b1;

                write_curr_data <= 18'd0;
                write_prev_data <= 18'd0;

                // start at bottom 
                read_prev_address <= 19'd0; 
                read_curr_address <= 19'd0;
				// idx[i] <= 18'd0;

                // set bottom boundary condition 
                u_bottom <= 18'd0; 
                
                // set u_curr
                u_curr[i] <= 18'b0;
                top_flag <= 1'b0;
				// pio_init_done[i] <= 1'b0;
            end
            else begin
                case (state)
                    // FIRST TWO STATES ARE OUR INIT STATE MACHINE // 
                    5'd0: begin // start writing initial values
                        if (addr_idx >= 19'd169) begin // once we are done writing all rows start processing
                            addr_idx <= 19'd0;
                            write_prev_address <= 19'd0;
                            write_curr_address <= 19'd0;
                            write_curr_en   <= 1'b0;
                            write_prev_en   <= 1'b0;
                            // u_curr[i] <= u_top;
                            state <= 5'd2;
                        end
                        else begin
                        write_curr_en <= 1'b1;
                        write_prev_en <= 1'b1;
						
                        // if ( i <= 19'd85 ) begin
                        //     if (addr_idx < i) begin
                        //         write_prev_data <= write_prev_data + 18'b0_00000000100000000;
                        //         write_curr_data <= write_curr_data + 18'b0_00000000100000000;
                        //     end
                        //     else if ((19'd169 - addr_idx) <= i) begin
                        //         write_prev_data <= write_prev_data - 18'b0_00000000100000000;
                        //         write_curr_data <= write_curr_data - 18'b0_00000000100000000;
                        //     end
                        // end
                        // else if (i > 19'd85) begin
                        //     if (i <= (19'd169 - addr_idx)) begin
                        //         write_prev_data <= write_prev_data + 18'b0_00000000100000000;
                        //         write_curr_data <= write_curr_data + 18'b0_00000000100000000;
                        //     end
                        //     else if (i < addr_idx) begin
                        //         write_prev_data <= write_prev_data - 18'b0_00000000100000000;
                        //         write_curr_data <= write_curr_data - 18'b0_00000000100000000;
                        //     end
                        // end

                        if ( i <= 19'd80 ) begin
							if (addr_idx < half_num_rows) begin
								// if (i == 19'd0 || addr_idx == 19'd0) begin
								// 	write_prev_data <= 18'd0;
                        		// 	write_curr_data <= 18'd0;
								// end
								if (i > addr_idx) begin
									write_prev_data <= write_prev_data + pio_step_y;
                        			write_curr_data <= write_prev_data + pio_step_y;
								end
								else if (i <= addr_idx) begin
									write_prev_data <= write_prev_data + pio_step_x;
                        			write_curr_data <= write_prev_data + pio_step_x;
								end
							end
							else if (addr_idx >= half_num_rows) begin
								// if (i == 19'd0 || addr_idx == (num_rows - 19'd1)) begin
								// 	write_prev_data <= 18'd0;
                        		// 	write_curr_data <= 18'd0;
								// end
								if ((i + addr_idx - half_num_rows - 19'd1) < half_num_rows) begin
									write_prev_data <= write_prev_data + pio_step_x;
                        			write_curr_data <= write_prev_data + pio_step_x;
								end
								else if ((i + addr_idx - half_num_rows - 19'd1) >= half_num_rows) begin
									write_prev_data <= write_prev_data - pio_step_y;
                        			write_curr_data <= write_prev_data - pio_step_y;
								end
							end
							// else begin
							// 	write_prev_data <= write_prev_data;
							// 	write_curr_data <= write_prev_data;
							// end

						end
						else if (i > 19'd80) begin
							if (addr_idx < half_num_rows) begin
								// if (i == 19'd159 || addr_idx == 0) begin
								// 	write_prev_data <= 18'd0;
                        		// 	write_curr_data <= 18'd0;
								// end
								if ((addr_idx + i - 19'd79) < half_num_rows) begin
									write_prev_data <= write_prev_data + pio_step_y;
                        			write_curr_data <= write_prev_data + pio_step_y;
								end
								else if ((addr_idx + i - 19'd79) >= half_num_rows) begin
									write_prev_data <= write_prev_data - pio_step_x;
                        			write_curr_data <= write_prev_data - pio_step_x;
								end
							end
							else if (addr_idx >= half_num_rows) begin
								// if (i == 19'd159 || addr_idx == (num_rows - 19'd1)) begin
								// 	write_prev_data <= 18'd0;
                        		// 	write_curr_data <= 18'd0;
								// end
								if (i < addr_idx) begin
									write_prev_data <= write_prev_data - pio_step_x;
                        			write_curr_data <= write_prev_data - pio_step_x;
								end
								else if (i >= addr_idx) begin
									write_prev_data <= write_prev_data - pio_step_y;
                        			write_curr_data <= write_prev_data - pio_step_y;
								end

							end
							// else begin
							// 	write_prev_data <= write_prev_data;
							// 	write_curr_data <= write_prev_data;
							// end

						end


                        
                            addr_idx <= addr_idx + 19'd1;
							write_prev_address <= write_prev_address + 19'd1;
							write_curr_address <= write_curr_address + 19'd1;
                            state <= 5'd0;
                        end
                        // idx   <= idx + 6'd1;
                            
                        
                    end
                    // 5'd1: begin // second init state
                    //     state <= 5'd0;
                    // end

                    // START OF OUR REGULAR STATE MACHINE // 
                    5'd2: begin // wait for one cycle for our read data to come back for u_curr
                        write_curr_en   <= 1'b0;
                        write_prev_en   <= 1'b0;
                        state           <= 5'd3;
                    end

                    5'd3: begin // when we loop back to the bottom, we have to read u_curr and u_top from m10k
                        u_curr[i]         <= u_top;
                        read_curr_address <= read_curr_address + 19'd1;
                        state             <= 5'd4;
                    end


                    5'd4: begin // wait one cycle for read data to come back for u_top
                        write_curr_en   <= 1'b0;
                        write_prev_en   <= 1'b0;
                        state 			<= 5'd5;
                    end

                    5'd5: begin
                        // write u_next to n m10k (update NEXT timestep for this node)
                        write_curr_en   <= 1'b1;
                        write_curr_data <= u_next;

                        // write u_curr to n-1 m10k (update PREV timestep for this node)
                        write_prev_en   <= 1'b1;  
                        write_prev_data <= u_curr[i];

                        // move up one node (in CURR timestep)
                        u_curr[i]       <= u_top;  
                        u_bottom        <= u_curr[i]; 

						if (read_prev_address == 19'd85) begin
							u_center[i] <= u_curr[i];
						end

                        state <= 5'd6;
                    end

                    5'd6: begin
                        write_curr_en   <= 1'b0;
                        write_prev_en   <= 1'b0;
                        
                        // state transition 
                        if (top_flag) begin // we've finished the column
							if (audio_request) begin // wait for audio bus master state machine to send a request for a new iteration 

								// restart from the bottom!

								u_bottom <= 18'd0; // set bottom boundary condition

								top_flag <= 1'b0;
								// audio_request_ack[i] <= 1'b0;

								write_curr_address <= 19'd0;
								write_prev_address <= 19'd0;

								read_prev_address <= 19'd0;
								// idx[i] <= 18'd0;
								addr_idx <= 19'd0;

								state <= 5'd3;
							end
							else begin
								state <= 5'd6;
							end
                        end
                        else begin
                            // move up one node (in CURR timestep)
                            
                            write_prev_address <= write_prev_address + 19'd1;
                            write_curr_address <= write_curr_address + 19'd1;

                            read_prev_address <= read_prev_address + 19'd1;
                            read_curr_address <= read_curr_address + 19'd1;

							addr_idx <= addr_idx + 19'd1;
									//  idx[i] <= idx[i] + 18'd1;

                            if (read_prev_address == 19'd168) begin
                                top_flag <= 1'b1;
                                read_curr_address <= 19'd0;
                            end

                            state <= 5'd4;
                        end
                        
                    end

                endcase
            end

        end
    end
endgenerate

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
    assign int_mid = (rho_mult + (u_curr << 1) - u_prev + (u_prev >>> 10));
    assign u_next = int_mid - (int_mid >>> 10);

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
    reg [17:0] mem [169:0]  /* synthesis ramstyle = "no_rw_check, M10K" */;
	// reg [7:0] mem [153600:0]; // 2 solvers
	// reg [7:0] mem [76800:0]; // 4 solvers
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
        q <= mem[read_address]; // q doesn't get d in this clock cycle
    end
endmodule


module DE1_SoC_Computer (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	CLOCK_50,
	CLOCK2_50,
	CLOCK3_50,
	CLOCK4_50,

	// ADC
	ADC_CS_N,
	ADC_DIN,
	ADC_DOUT,
	ADC_SCLK,

	// Audio
	AUD_ADCDAT,
	AUD_ADCLRCK,
	AUD_BCLK,
	AUD_DACDAT,
	AUD_DACLRCK,
	AUD_XCK,

	// SDRAM
	DRAM_ADDR,
	DRAM_BA,
	DRAM_CAS_N,
	DRAM_CKE,
	DRAM_CLK,
	DRAM_CS_N,
	DRAM_DQ,
	DRAM_LDQM,
	DRAM_RAS_N,
	DRAM_UDQM,
	DRAM_WE_N,

	// I2C Bus for Configuration of the Audio and Video-In Chips
	FPGA_I2C_SCLK,
	FPGA_I2C_SDAT,

	// 40-Pin Headers
	GPIO_0,
	GPIO_1,
	
	// Seven Segment Displays
	HEX0,
	HEX1,
	HEX2,
	HEX3,
	HEX4,
	HEX5,

	// IR
	IRDA_RXD,
	IRDA_TXD,

	// Pushbuttons
	KEY,

	// LEDs
	LEDR,

	// PS2 Ports
	PS2_CLK,
	PS2_DAT,
	
	PS2_CLK2,
	PS2_DAT2,

	// Slider Switches
	SW,

	// Video-In
	TD_CLK27,
	TD_DATA,
	TD_HS,
	TD_RESET_N,
	TD_VS,

	// VGA
	VGA_B,
	VGA_BLANK_N,
	VGA_CLK,
	VGA_G,
	VGA_HS,
	VGA_R,
	VGA_SYNC_N,
	VGA_VS,

	////////////////////////////////////
	// HPS Pins
	////////////////////////////////////
	
	// DDR3 SDRAM
	HPS_DDR3_ADDR,
	HPS_DDR3_BA,
	HPS_DDR3_CAS_N,
	HPS_DDR3_CKE,
	HPS_DDR3_CK_N,
	HPS_DDR3_CK_P,
	HPS_DDR3_CS_N,
	HPS_DDR3_DM,
	HPS_DDR3_DQ,
	HPS_DDR3_DQS_N,
	HPS_DDR3_DQS_P,
	HPS_DDR3_ODT,
	HPS_DDR3_RAS_N,
	HPS_DDR3_RESET_N,
	HPS_DDR3_RZQ,
	HPS_DDR3_WE_N,

	// Ethernet
	HPS_ENET_GTX_CLK,
	HPS_ENET_INT_N,
	HPS_ENET_MDC,
	HPS_ENET_MDIO,
	HPS_ENET_RX_CLK,
	HPS_ENET_RX_DATA,
	HPS_ENET_RX_DV,
	HPS_ENET_TX_DATA,
	HPS_ENET_TX_EN,

	// Flash
	HPS_FLASH_DATA,
	HPS_FLASH_DCLK,
	HPS_FLASH_NCSO,

	// Accelerometer
	HPS_GSENSOR_INT,
		
	// General Purpose I/O
	HPS_GPIO,
		
	// I2C
	HPS_I2C_CONTROL,
	HPS_I2C1_SCLK,
	HPS_I2C1_SDAT,
	HPS_I2C2_SCLK,
	HPS_I2C2_SDAT,

	// Pushbutton
	HPS_KEY,

	// LED
	HPS_LED,
		
	// SD Card
	HPS_SD_CLK,
	HPS_SD_CMD,
	HPS_SD_DATA,

	// SPI
	HPS_SPIM_CLK,
	HPS_SPIM_MISO,
	HPS_SPIM_MOSI,
	HPS_SPIM_SS,

	// UART
	HPS_UART_RX,
	HPS_UART_TX,

	// USB
	HPS_CONV_USB_N,
	HPS_USB_CLKOUT,
	HPS_USB_DATA,
	HPS_USB_DIR,
	HPS_USB_NXT,
	HPS_USB_STP
);

//=======================================================
//  PARAMETER declarations
//=======================================================


//=======================================================
//  PORT declarations
//=======================================================

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pins
input						CLOCK_50;
input						CLOCK2_50;
input						CLOCK3_50;
input						CLOCK4_50;

// ADC
inout						ADC_CS_N;
output					ADC_DIN;
input						ADC_DOUT;
output					ADC_SCLK;

// Audio
input						AUD_ADCDAT;
inout						AUD_ADCLRCK;
inout						AUD_BCLK;
output					AUD_DACDAT;
inout						AUD_DACLRCK;
output					AUD_XCK;

// SDRAM
output 		[12: 0]	DRAM_ADDR;
output		[ 1: 0]	DRAM_BA;
output					DRAM_CAS_N;
output					DRAM_CKE;
output					DRAM_CLK;
output					DRAM_CS_N;
inout			[15: 0]	DRAM_DQ;
output					DRAM_LDQM;
output					DRAM_RAS_N;
output					DRAM_UDQM;
output					DRAM_WE_N;

// I2C Bus for Configuration of the Audio and Video-In Chips
output					FPGA_I2C_SCLK;
inout						FPGA_I2C_SDAT;

// 40-pin headers
inout			[35: 0]	GPIO_0;
inout			[35: 0]	GPIO_1;

// Seven Segment Displays
output		[ 6: 0]	HEX0;
output		[ 6: 0]	HEX1;
output		[ 6: 0]	HEX2;
output		[ 6: 0]	HEX3;
output		[ 6: 0]	HEX4;
output		[ 6: 0]	HEX5;

// IR
input						IRDA_RXD;
output					IRDA_TXD;

// Pushbuttons
input			[ 3: 0]	KEY;

// LEDs
output		[ 9: 0]	LEDR;

// PS2 Ports
inout						PS2_CLK;
inout						PS2_DAT;

inout						PS2_CLK2;
inout						PS2_DAT2;

// Slider Switches
input			[ 9: 0]	SW;

// Video-In
input						TD_CLK27;
input			[ 7: 0]	TD_DATA;
input						TD_HS;
output					TD_RESET_N;
input						TD_VS;

// VGA
output		[ 7: 0]	VGA_B;
output					VGA_BLANK_N;
output					VGA_CLK;
output		[ 7: 0]	VGA_G;
output					VGA_HS;
output		[ 7: 0]	VGA_R;
output					VGA_SYNC_N;
output					VGA_VS;



////////////////////////////////////
// HPS Pins
////////////////////////////////////
	
// DDR3 SDRAM
output		[14: 0]	HPS_DDR3_ADDR;
output		[ 2: 0]  HPS_DDR3_BA;
output					HPS_DDR3_CAS_N;
output					HPS_DDR3_CKE;
output					HPS_DDR3_CK_N;
output					HPS_DDR3_CK_P;
output					HPS_DDR3_CS_N;
output		[ 3: 0]	HPS_DDR3_DM;
inout			[31: 0]	HPS_DDR3_DQ;
inout			[ 3: 0]	HPS_DDR3_DQS_N;
inout			[ 3: 0]	HPS_DDR3_DQS_P;
output					HPS_DDR3_ODT;
output					HPS_DDR3_RAS_N;
output					HPS_DDR3_RESET_N;
input						HPS_DDR3_RZQ;
output					HPS_DDR3_WE_N;

// Ethernet
output					HPS_ENET_GTX_CLK;
inout						HPS_ENET_INT_N;
output					HPS_ENET_MDC;
inout						HPS_ENET_MDIO;
input						HPS_ENET_RX_CLK;
input			[ 3: 0]	HPS_ENET_RX_DATA;
input						HPS_ENET_RX_DV;
output		[ 3: 0]	HPS_ENET_TX_DATA;
output					HPS_ENET_TX_EN;

// Flash
inout			[ 3: 0]	HPS_FLASH_DATA;
output					HPS_FLASH_DCLK;
output					HPS_FLASH_NCSO;

// Accelerometer
inout						HPS_GSENSOR_INT;

// General Purpose I/O
inout			[ 1: 0]	HPS_GPIO;

// I2C
inout						HPS_I2C_CONTROL;
inout						HPS_I2C1_SCLK;
inout						HPS_I2C1_SDAT;
inout						HPS_I2C2_SCLK;
inout						HPS_I2C2_SDAT;

// Pushbutton
inout						HPS_KEY;

// LED
inout						HPS_LED;

// SD Card
output					HPS_SD_CLK;
inout						HPS_SD_CMD;
inout			[ 3: 0]	HPS_SD_DATA;

// SPI
output					HPS_SPIM_CLK;
input						HPS_SPIM_MISO;
output					HPS_SPIM_MOSI;
inout						HPS_SPIM_SS;

// UART
input						HPS_UART_RX;
output					HPS_UART_TX;

// USB
inout						HPS_CONV_USB_N;
input						HPS_USB_CLKOUT;
inout			[ 7: 0]	HPS_USB_DATA;
input						HPS_USB_DIR;
input						HPS_USB_NXT;
output					HPS_USB_STP;

//=======================================================
//  REG/WIRE declarations
//=======================================================

wire			[15: 0]	hex3_hex0;
//wire			[15: 0]	hex5_hex4;

//assign HEX0 = ~hex3_hex0[ 6: 0]; // hex3_hex0[ 6: 0]; 
//assign HEX1 = ~hex3_hex0[14: 8];
//assign HEX2 = ~hex3_hex0[22:16];
//assign HEX3 = ~hex3_hex0[30:24];
assign HEX4 = 7'b1111111;
assign HEX5 = 7'b1111111;
assign HEX3 = 7'b1111111;
assign HEX2 = 7'b1111111;
assign HEX1 = 7'b1111111;
assign HEX0 = 7'b1111111;

//=======================================================
//  DRUM SYNTH
//=======================================================

// pll fast clock 
// M10k memory clock
wire 					pll_fast_clk_100 ;
wire 					pll_fast_clk_100_locked ;

// pio ports for initial values 
// TODO: make sure to set these to wires when we instantiate the pio ports!!
wire [31:0] pio_init;
reg  [29:0] pio_init_done;
wire        pio_init_val;
reg audio_request; 
reg audio_request_ack [29:0]; // we only use the [15], but use an array so we can reset in the generate block 



// BOTTOM IDX = 0
// TOP IDX = 29 (N-1)

// register this and update every iteration
// this has to be outside our generate so we can connect between columns
reg [17:0] u_curr [29:0];
reg [17:0] u_center [29:0];


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
        .a   (u_center[15]>>>4),
        .b   (u_center[15]>>>4)
    );

//=======================================================
//  DRUM SYNTH STATE MACHINE
//=======================================================

// reg [29:0] idx;

always @(posedge CLOCK_50) begin
	if (~KEY[0]) begin
		// rho
		rho_eff <= 18'b0_01000000000000000;
	end
	else begin
		// if (idx[15] == 19'd15) begin // output the center node!
		// 	u_center <= u_curr[15];
		// end

		if (max_rho < (rho_0 + rho_gtension)) begin
        	rho_eff <= rho_0 + rho_gtension;
		end
		else begin
			rho_eff <= max_rho;
		end
	end
end



generate
    genvar i;
    for ( i = 0; i < 30; i = i + 1 ) begin: gen1

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
            .clk           (CLOCK_50)
        );

        M10K_1000_8 M10k_u_curr ( 
            .q             (u_top),  // we always read out u_top from this m10k
            .d             (write_curr_data), // we always write u_next to this m10k
            .write_address (write_curr_address),
            .read_address  (read_curr_address),
            .we            (write_curr_en),
            .clk           (CLOCK_50)
        );

        drum_syn drum_syn_inst (
                    .rho          (rho_eff), // 0.0625 = 18'b0_00010000000000000
                    .u_top        ((top_flag) ? (18'd0) : u_top), // if we're at the top, 0 boundary condition
                    .u_bottom     (u_bottom), // if we're at the bottom, 0 boundary condition (set in state machine reset)
                    .u_left       ((i == 0) ? (18'd0) : u_curr[i-1]),
                    .u_right      ((i == 29) ? (18'd0) : u_curr[i+1]),
                    .u_prev       (u_prev),
                    .u_curr       (u_curr[i]),
                    .u_next       (u_next)
        ); 

        always @(posedge CLOCK_50) begin
			// audio_request_ack[i] <= audio_request;

            if (~KEY[0]) begin
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
                u_curr[i] <= 18'b0_00000000000100000;
                top_flag <= 1'b0;
				// pio_init_done[i] <= 1'b0;
            end
            else begin
                case (state)
                    // FIRST TWO STATES ARE OUR INIT STATE MACHINE // 
                    5'd0: begin // start writing initial values
                        if (addr_idx >= 19'd29) begin // once we are done writing all rows start processing
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
						
                        if ( i <= 19'd15 ) begin
                            if (addr_idx < i) begin
                                write_prev_data <= write_prev_data + 18'b0_00000000100000000;
                                write_curr_data <= write_curr_data + 18'b0_00000000100000000;
                            end
                            else if ((19'd29 - addr_idx) <= i) begin
                                write_prev_data <= write_prev_data - 18'b0_00000000100000000;
                                write_curr_data <= write_curr_data - 18'b0_00000000100000000;
                            end
                        end
                        else if (i > 19'd15) begin
                            if (i <= (19'd29 - addr_idx)) begin
                                write_prev_data <= write_prev_data + 18'b0_00000000100000000;
                                write_curr_data <= write_curr_data + 18'b0_00000000100000000;
                            end
                            else if (i < addr_idx) begin
                                write_prev_data <= write_prev_data - 18'b0_00000000100000000;
                                write_curr_data <= write_curr_data - 18'b0_00000000100000000;
                            end
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

						if (read_prev_address == 19'd15) begin
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
    end
endgenerate



//=======================================================
// Bus controller for AVALON bus-master
//=======================================================
// computes DDS for sine wave and fills audio FIFO

reg [31:0] bus_addr ; // Avalon address
// see 
// ftp://ftp.altera.com/up/pub/Altera_Material/15.1/University_Program_IP_Cores/Audio_Video/Audio.pdf
// for addresses
wire [31:0] audio_base_address = 32'h00003040 ;  // Avalon address
wire [31:0] audio_fifo_address = 32'h00003044 ;  // Avalon address +4 offset
wire [31:0] audio_left_address = 32'h00003048 ;  // Avalon address +8
wire [31:0] audio_right_address = 32'h0000304c ;  // Avalon address +12
reg [3:0] bus_byte_enable ; // four bit byte read/write mask
reg bus_read  ;       // high when requesting data
reg bus_write ;      //  high when writing data
reg [31:0] bus_write_data ; //  data to send to Avalog bus
wire bus_ack  ;       //  Avalon bus raises this when done
wire [31:0] bus_read_data ; // data from Avalon bus
reg [30:0] timer ;
reg [3:0] state ;
// wire state_clock ;

// current free words in audio interface
reg [7:0] fifo_space ;
// debug check of space
assign LEDR = fifo_space ;

// use 4-byte-wide bus-master	 
//assign bus_byte_enable = 4'b1111;

// DDS signals
// reg [31:0] dds_accum ;
// DDS LUT
// wire [15:0] sine_out ;
// sync_rom sineTable(CLOCK_50, dds_accum[31:24], sine_out);

// get some signals exposed
// connect bus master signals to i/o for probes
assign GPIO_0[0] = bus_write ;
assign GPIO_0[1] = bus_read ;
assign GPIO_0[2] = bus_ack ;
//assign GPIO_0[3] = ??? ;

assign GPIO_0[3] = audio_request;

always @(posedge CLOCK_50) begin //CLOCK_50

	// connect synchronization signals
	// audio_request <= audio_request_ack[15];

	// reset state machine and read/write controls
	if (~KEY[0]) begin
		state <= 0 ;
		bus_read <= 0 ; // set to one if a read opeation from bus
		bus_write <= 0 ; // set to one if a write operation to bus
		timer <= 0;
	end
	else begin
		// timer just for deubgging
		timer <= timer + 1;
	end
	
	// set up read FIFO available space
	if (state==4'd0) begin
		bus_addr <= audio_fifo_address ;
		bus_read <= 1'b1 ;
		bus_byte_enable <= 4'b1111;
		state <= 4'd1 ; // wait for read ACK
	end
	
	// wait for read ACK and read the fifo available
	// bus ACK is high when data is available
	if (state==4'd1 && bus_ack==1) begin
		state <= 4'd2 ; //4'd2
		// FIFO space is in high byte
		fifo_space <= (bus_read_data>>24) ;
		// end the read
		bus_read <= 1'b0 ;
	end
	
	// When there is room in the FIFO
	// -- compute next DDS sine sample
	// -- start write to fifo for each channel
	// -- first the left channel
	if (state==4'd2 && fifo_space>8'd2) begin // 
		state <= 4'd3;	
		// IF SW=10'h200 
		// and Fout = (sample_rate)/(2^32)*{SW[9:0], 16'b0}
		// then Fout=48000/(2^32)*(2^25) = 375 Hz
		// dds_accum <= dds_accum + {SW[9:0], 16'b0} ;
		// convert 16-bit table to 32-bit format
		bus_write_data <= (u_center[15] << 14) ;
		bus_addr <= audio_left_address ;
		bus_byte_enable <= 4'b1111;
		bus_write <= 1'b1 ;
	end	
	// if no space, try again later
	else if (state==4'd2 && fifo_space<=8'd2) begin
		state <= 4'b0 ;
	end
	
	// detect bus-transaction-complete ACK 
	// for left channel write
	// You MUST do this check
	if (state==4'd3 && bus_ack==1) begin
		state <= 4'd4 ;
		bus_write <= 0;
	end
	
	// -- now the right channel
	if (state==4'd4) begin // 
		state <= 4'd5;	
		bus_write_data <= (u_center[15] << 14) ;
		bus_addr <= audio_right_address ;
		bus_write <= 1'b1 ;
		audio_request <= 1'b1; // send request to drum for a new value
	end	
	if (state == 4'd5) begin
		audio_request <= 1'b0; // send request to drum for a new value
	end
	
	// detect bus-transaction-complete ACK
	// for right channel write
	// You MUST do this check
	if (state==4'd5 && bus_ack==1) begin
		state <= 4'd0 ;
		bus_write <= 0;
	end
	
end


//=======================================================
//  Structural coding
//=======================================================

Computer_System The_System (
	////////////////////////////////////
	// FPGA Side
	////////////////////////////////////

	// Global signals
	.system_pll_ref_clk_clk					(CLOCK_50),
	.system_pll_ref_reset_reset			(1'b0),
	// .sdram_clk_clk								(state_clock),

	// AV Config
	.av_config_SCLK							(FPGA_I2C_SCLK),
	.av_config_SDAT							(FPGA_I2C_SDAT),

	// Audio Subsystem
	.audio_pll_ref_clk_clk					(CLOCK3_50),
	.audio_pll_ref_reset_reset				(1'b0),
	.audio_clk_clk								(AUD_XCK),
	.audio_ADCDAT								(AUD_ADCDAT),
	.audio_ADCLRCK								(AUD_ADCLRCK),
	.audio_BCLK									(AUD_BCLK),
	.audio_DACDAT								(AUD_DACDAT),
	.audio_DACLRCK								(AUD_DACLRCK),

	// bus-master state machine interface
	.bus_master_audio_external_interface_address     (bus_addr),     
	.bus_master_audio_external_interface_byte_enable (bus_byte_enable), 
	.bus_master_audio_external_interface_read        (bus_read),        
	.bus_master_audio_external_interface_write       (bus_write),       
	.bus_master_audio_external_interface_write_data  (bus_write_data),  
	.bus_master_audio_external_interface_acknowledge (bus_ack),                                  
	.bus_master_audio_external_interface_read_data   (bus_read_data),   
	
	
	////////////////////////////////////
	// HPS Side
	////////////////////////////////////
	// DDR3 SDRAM
	.memory_mem_a			(HPS_DDR3_ADDR),
	.memory_mem_ba			(HPS_DDR3_BA),
	.memory_mem_ck			(HPS_DDR3_CK_P),
	.memory_mem_ck_n		(HPS_DDR3_CK_N),
	.memory_mem_cke		(HPS_DDR3_CKE),
	.memory_mem_cs_n		(HPS_DDR3_CS_N),
	.memory_mem_ras_n		(HPS_DDR3_RAS_N),
	.memory_mem_cas_n		(HPS_DDR3_CAS_N),
	.memory_mem_we_n		(HPS_DDR3_WE_N),
	.memory_mem_reset_n	(HPS_DDR3_RESET_N),
	.memory_mem_dq			(HPS_DDR3_DQ),
	.memory_mem_dqs		(HPS_DDR3_DQS_P),
	.memory_mem_dqs_n		(HPS_DDR3_DQS_N),
	.memory_mem_odt		(HPS_DDR3_ODT),
	.memory_mem_dm			(HPS_DDR3_DM),
	.memory_oct_rzqin		(HPS_DDR3_RZQ),
		  
	// Ethernet
	.hps_io_hps_io_gpio_inst_GPIO35	(HPS_ENET_INT_N),
	.hps_io_hps_io_emac1_inst_TX_CLK	(HPS_ENET_GTX_CLK),
	.hps_io_hps_io_emac1_inst_TXD0	(HPS_ENET_TX_DATA[0]),
	.hps_io_hps_io_emac1_inst_TXD1	(HPS_ENET_TX_DATA[1]),
	.hps_io_hps_io_emac1_inst_TXD2	(HPS_ENET_TX_DATA[2]),
	.hps_io_hps_io_emac1_inst_TXD3	(HPS_ENET_TX_DATA[3]),
	.hps_io_hps_io_emac1_inst_RXD0	(HPS_ENET_RX_DATA[0]),
	.hps_io_hps_io_emac1_inst_MDIO	(HPS_ENET_MDIO),
	.hps_io_hps_io_emac1_inst_MDC		(HPS_ENET_MDC),
	.hps_io_hps_io_emac1_inst_RX_CTL	(HPS_ENET_RX_DV),
	.hps_io_hps_io_emac1_inst_TX_CTL	(HPS_ENET_TX_EN),
	.hps_io_hps_io_emac1_inst_RX_CLK	(HPS_ENET_RX_CLK),
	.hps_io_hps_io_emac1_inst_RXD1	(HPS_ENET_RX_DATA[1]),
	.hps_io_hps_io_emac1_inst_RXD2	(HPS_ENET_RX_DATA[2]),
	.hps_io_hps_io_emac1_inst_RXD3	(HPS_ENET_RX_DATA[3]),

	// Flash
	.hps_io_hps_io_qspi_inst_IO0	(HPS_FLASH_DATA[0]),
	.hps_io_hps_io_qspi_inst_IO1	(HPS_FLASH_DATA[1]),
	.hps_io_hps_io_qspi_inst_IO2	(HPS_FLASH_DATA[2]),
	.hps_io_hps_io_qspi_inst_IO3	(HPS_FLASH_DATA[3]),
	.hps_io_hps_io_qspi_inst_SS0	(HPS_FLASH_NCSO),
	.hps_io_hps_io_qspi_inst_CLK	(HPS_FLASH_DCLK),

	// Accelerometer
	.hps_io_hps_io_gpio_inst_GPIO61	(HPS_GSENSOR_INT),

	//.adc_sclk                        (ADC_SCLK),
	//.adc_cs_n                        (ADC_CS_N),
	//.adc_dout                        (ADC_DOUT),
	//.adc_din                         (ADC_DIN),

	// General Purpose I/O
	.hps_io_hps_io_gpio_inst_GPIO40	(HPS_GPIO[0]),
	.hps_io_hps_io_gpio_inst_GPIO41	(HPS_GPIO[1]),

	// I2C
	.hps_io_hps_io_gpio_inst_GPIO48	(HPS_I2C_CONTROL),
	.hps_io_hps_io_i2c0_inst_SDA		(HPS_I2C1_SDAT),
	.hps_io_hps_io_i2c0_inst_SCL		(HPS_I2C1_SCLK),
	.hps_io_hps_io_i2c1_inst_SDA		(HPS_I2C2_SDAT),
	.hps_io_hps_io_i2c1_inst_SCL		(HPS_I2C2_SCLK),

	// Pushbutton
	.hps_io_hps_io_gpio_inst_GPIO54	(HPS_KEY),

	// LED
	.hps_io_hps_io_gpio_inst_GPIO53	(HPS_LED),

	// SD Card
	.hps_io_hps_io_sdio_inst_CMD	(HPS_SD_CMD),
	.hps_io_hps_io_sdio_inst_D0	(HPS_SD_DATA[0]),
	.hps_io_hps_io_sdio_inst_D1	(HPS_SD_DATA[1]),
	.hps_io_hps_io_sdio_inst_CLK	(HPS_SD_CLK),
	.hps_io_hps_io_sdio_inst_D2	(HPS_SD_DATA[2]),
	.hps_io_hps_io_sdio_inst_D3	(HPS_SD_DATA[3]),

	// SPI
	.hps_io_hps_io_spim1_inst_CLK		(HPS_SPIM_CLK),
	.hps_io_hps_io_spim1_inst_MOSI	(HPS_SPIM_MOSI),
	.hps_io_hps_io_spim1_inst_MISO	(HPS_SPIM_MISO),
	.hps_io_hps_io_spim1_inst_SS0		(HPS_SPIM_SS),

	// UART
	.hps_io_hps_io_uart0_inst_RX	(HPS_UART_RX),
	.hps_io_hps_io_uart0_inst_TX	(HPS_UART_TX),

	// USB
	.hps_io_hps_io_gpio_inst_GPIO09	(HPS_CONV_USB_N),
	.hps_io_hps_io_usb1_inst_D0		(HPS_USB_DATA[0]),
	.hps_io_hps_io_usb1_inst_D1		(HPS_USB_DATA[1]),
	.hps_io_hps_io_usb1_inst_D2		(HPS_USB_DATA[2]),
	.hps_io_hps_io_usb1_inst_D3		(HPS_USB_DATA[3]),
	.hps_io_hps_io_usb1_inst_D4		(HPS_USB_DATA[4]),
	.hps_io_hps_io_usb1_inst_D5		(HPS_USB_DATA[5]),
	.hps_io_hps_io_usb1_inst_D6		(HPS_USB_DATA[6]),
	.hps_io_hps_io_usb1_inst_D7		(HPS_USB_DATA[7]),
	.hps_io_hps_io_usb1_inst_CLK		(HPS_USB_CLKOUT),
	.hps_io_hps_io_usb1_inst_STP		(HPS_USB_STP),
	.hps_io_hps_io_usb1_inst_DIR		(HPS_USB_DIR),
	.hps_io_hps_io_usb1_inst_NXT		(HPS_USB_NXT),
	.pio_init_export                 (pio_init),    //                            pio_init.export
	.pio_init_done_export            (pio_init_done[0]),//                       pio_init_done.export
	.pio_init_val_export             (pio_init_val),
	.pio_reset_export                (~KEY[0]),
	.pll_fast_clk_100_outclk0_clk    (pll_fast_clk_100),                    //            pll_fast_clk_100_outclk0.clk
	.pll_fast_clk_100_locked_export  (pll_fast_clk_100_locked)                   //             pll_fast_clk_100_locked.export
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
    reg [17:0] mem [29:0]  /* synthesis ramstyle = "no_rw_check, M10K" */;
	// reg [7:0] mem [153600:0]; // 2 solvers
	// reg [7:0] mem [76800:0]; // 4 solvers
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
        q <= mem[read_address]; // q doesn't get d in this clock cycle
    end
endmodule
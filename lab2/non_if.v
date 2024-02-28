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

assign HEX4 = 7'b1111111;
assign HEX5 = 7'b1111111;

HexDigit Digit0(HEX0, hex3_hex0[3:0]);
HexDigit Digit1(HEX1, hex3_hex0[7:4]);
HexDigit Digit2(HEX2, hex3_hex0[11:8]);
HexDigit Digit3(HEX3, hex3_hex0[15:12]);


//=======================================================
// SRAM/VGA state machine
//=======================================================
// --Check for sram address=0 nonzero, which means that
//   HPS wrote some new data.
//
// --Read sram address 1 and 2 to get x1, y1 
//   left-most x, upper-most y
// --Read sram address 3 and 4 to get x2, y2
//   right-most x, lower-most y
// --Read sram address 5 to get color
// --write a rectangle to VGA
//
// --clear sram address=0 to signal HPS
//=======================================================
// Controls for Qsys sram slave exported in system module
//=======================================================
wire [31:0] sram_readdata ;
reg [31:0] data_buffer, sram_writedata ;
reg [7:0] sram_address; 
reg sram_write ;
wire sram_clken = 1'b1;
wire sram_chipselect = 1'b1;

// rectangle corners
reg [9:0] x1, y1, x2, y2 ;
reg [31:0] timer ; // may need to throttle write-rate
//=======================================================
// Controls for VGA memory
//=======================================================
wire [31:0] vga_out_base_address = 32'h0000_0000 ;  // vga base addr
reg [7:0] vga_sram_writedata ;
reg [31:0] vga_sram_address; 
reg vga_sram_write ;
wire vga_sram_clken = 1'b1;
wire vga_sram_chipselect = 1'b1;

//=======================================================
// pixel address is
wire [9:0] vga_x_cood [1:0];
wire [9:0] vga_y_cood [1:0];
wire [7:0] pixel_color [1:0];

//=======================================================

wire signed [26:0] c_r [1:0]; //27'b1111_00000000000000000000000
wire signed [26:0] c_i [1:0]; // 27'b0000_00000000000000000000000

wire [15:0] max_iter;
assign max_iter = 16'd1000; //TODO: eventually this will connect to the arm!

wire [15:0] iter [1:0];
wire done [1:0];

wire calc_done [1:0];
wire reset_solver [1:0];
reg draw_done [1:0];
wire draw_done_out [1:0];

//=======================================================
// ARBITER MODULE 
//=======================================================

always @(posedge CLOCK_50) begin

	// reset read/write controls
	if (~KEY[0]) begin
		vga_sram_write <= 1'b0 ; // set to on if a write operation to bus
		timer <= 0;
	end

	else begin
		// increment timer every cycle
		timer <= timer + 1'b1;

		casex({done[1],done[0]})
			2'bx1: begin
				vga_sram_write <= calc_done[0];
				vga_sram_address <= vga_out_base_address + {22'b0, vga_x_cood[0]} + ({22'b0,vga_y_cood[0]}*640); 
				vga_sram_writedata <= pixel_color[0];
	//			draw <= 2'b01;
				// done[0] <= 1'b0;
				draw_done[0] <= 1'b1;
			end
				
			2'b10: begin
				vga_sram_write <= calc_done[1];
				vga_sram_address <= vga_out_base_address + {22'b0, vga_x_cood[1]} + ({22'b0,vga_y_cood[1]}*640); 
				vga_sram_writedata <= pixel_color[1];
				draw_done[1] <= 1'b1;
			end
	//			draw <= 2'b10;
				// done[1] <= 1'b0;
				

			default: begin 
				vga_sram_write <= 1'b0;
				vga_sram_address <= vga_out_base_address + {22'b0, vga_x_cood[0]} + ({22'b0,vga_y_cood[0]}*640); 
				vga_sram_writedata <= pixel_color[0];
				draw_done[0] <= draw_done_out[0];
				draw_done[1] <= draw_done_out[1];
			end
		endcase
	end
end 

//=======================================================
// ITERATOR MODULE INSTANTIATIONS
//=======================================================
 
generate
	genvar i;

	for ( i = 0; i < 2; i=i+1) begin: gen1
		complex_iterator complex_iterator_inst (
			.clk          (CLOCK_50),
			.reset        (reset_solver[i]),
			.max_iter     (max_iter),

			.c_r          ( c_r[i]),
			.c_i          ( c_i[i]),

			.iter         (iter[i]),
			.done         (done[i])
		);

	 // solver one
		solver_state_machine solver_state_machine_inst ( 
			.clk 		(CLOCK_50),
			.reset		(~KEY[0]),
			.i  		(i),
			.done 		(done[i]),
			.iter 		(iter[i]),
			.max_iter 	(max_iter),
			.draw_done  (draw_done[i]),

			.c_r          (c_r[i]),
			.c_i          (c_i[i]),
			.pixel_color  (pixel_color[i]),
			.vga_x_cood	  (vga_x_cood[i]),
			.vga_y_cood	  (vga_y_cood[i]),
			.calc_done    (calc_done[i]),
			.reset_solver (reset_solver[i]),
			.draw_done_out(draw_done_out[i])

		);
	
	end

endgenerate



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

	// Slider Switches
	.slider_switches_export					(SW),

	// Pushbuttons (~KEY[3:0]),
	.pushbuttons_export						(~KEY[3:0]),

	// Expansion JP1
	//.expansion_jp1_export					({GPIO_0[35:19], GPIO_0[17], GPIO_0[15:3], GPIO_0[1]}),

	// Expansion JP2
	//.expansion_jp2_export					({GPIO_1[35:19], GPIO_1[17], GPIO_1[15:3], GPIO_1[1]}),

	// LEDs
	.leds_export								(LEDR),
	
	// Seven Segs
	.hex3_hex0_export							(hex3_hex0),
	//.hex5_hex4_export							(hex5_hex4),
	
	// PS2 Ports
	//.ps2_port_CLK								(PS2_CLK),
	//.ps2_port_DAT								(PS2_DAT),
	//.ps2_port_dual_CLK						(PS2_CLK2),
	//.ps2_port_dual_DAT						(PS2_DAT2),

	// IrDA
	//.irda_RXD									(IRDA_RXD),
	//.irda_TXD									(IRDA_TXD),

	// VGA Subsystem
	.vga_pll_ref_clk_clk 					(CLOCK2_50),
	.vga_pll_ref_reset_reset				(1'b0),
	.vga_CLK										(VGA_CLK),
	.vga_BLANK									(VGA_BLANK_N),
	.vga_SYNC									(VGA_SYNC_N),
	.vga_HS										(VGA_HS),
	.vga_VS										(VGA_VS),
	.vga_R										(VGA_R),
	.vga_G										(VGA_G),
	.vga_B										(VGA_B),
	
	// Video In Subsystem
//	.video_in_TD_CLK27 						(TD_CLK27),
//	.video_in_TD_DATA							(TD_DATA),
//	.video_in_TD_HS							(TD_HS),
//	.video_in_TD_VS							(TD_VS),
//	.video_in_clk27_reset					(),
//	.video_in_TD_RESET						(TD_RESET_N),
//	.video_in_overflow_flag					(),
	
	// SDRAM
//	.sdram_clk_clk								(DRAM_CLK),
   .sdram_addr									(DRAM_ADDR),
	.sdram_ba									(DRAM_BA),
	.sdram_cas_n								(DRAM_CAS_N),
	.sdram_cke									(DRAM_CKE),
	.sdram_cs_n									(DRAM_CS_N),
	.sdram_dq									(DRAM_DQ),
	.sdram_dqm									({DRAM_UDQM,DRAM_LDQM}),
	.sdram_ras_n								(DRAM_RAS_N),
	.sdram_we_n									(DRAM_WE_N),
	
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

	// SRAM shared block with HPS
	// .onchip_sram_s1_address               (sram_address),               
	// .onchip_sram_s1_clken                 (sram_clken),                 
	// .onchip_sram_s1_chipselect            (sram_chipselect),            
	// .onchip_sram_s1_write                 (sram_write),                 
	// .onchip_sram_s1_readdata              (sram_readdata),              
	// .onchip_sram_s1_writedata             (sram_writedata),             
	// .onchip_sram_s1_byteenable            (4'b1111), 

	//  sram to video
	.onchip_memory2_0_s1_address     (vga_sram_address),    
	.onchip_memory2_0_s1_clken       (vga_sram_clken),      
	.onchip_memory2_0_s1_chipselect  (vga_sram_chipselect), 
	.onchip_memory2_0_s1_write       (vga_sram_write),      
	.onchip_memory2_0_s1_readdata    (),   // never read from vga here
	.onchip_memory2_0_s1_writedata   (vga_sram_writedata),   

	// 50 MHz clock bridge
	.clock_bridge_0_in_clk_clk            (CLOCK_50) //(CLOCK_50), 
);


endmodule

/////////////////////////////////////////////////////
/////////////// OUR ITERATOR MODULE /////////////////
/////////////////////////////////////////////////////


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
    // wire        [15:0] counter;

    // top branch internal sigs

    wire signed [26:0] z_r_i_sq_sum;
    wire signed [26:0] z_r_tmp;
    wire signed [26:0] z_r_sq_tmp;

    // bottom branch internal sigs
    wire signed [26:0] z_r_i_mul;
    wire signed [26:0] z_i_tmp;
    wire signed [26:0] z_i_sq_tmp;

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
    

    // if sqr magnitude (z_out) is greater than 4 (2^2) or we exceed max iterations:

    assign done = (z_out > 27'b0100_00000000000000000000000) | (iter  > max_iter);
    
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


module solver_state_machine (
	input wire 		  clk,
	input wire		  reset,
	
	input wire		  i,
	input wire        done,
	input wire [15:0] iter,
	input wire [15:0] max_iter,
	input wire        draw_done,
	
	output wire [26:0] c_r,
	output wire [26:0] c_i,

	output wire [7:0] pixel_color,
	output wire [9:0] vga_x_cood,
	output wire [9:0] vga_y_cood,
	output wire       calc_done,
	output wire       reset_solver,
	output wire       draw_done_out
);

	reg [3:0] state;
	reg [9:0] vga_x_cood_reg;
	reg [9:0] vga_y_cood_reg;
	reg [7:0] pixel_color_reg;
	reg [26:0] c_r_reg;
	reg [26:0] c_i_reg;
	reg        calc_done_reg;
	reg        reset_solver_reg;
	reg        draw_done_out_reg;
	
	assign vga_x_cood = vga_x_cood_reg;
	assign vga_y_cood = vga_y_cood_reg;
	assign pixel_color = pixel_color_reg;
	
	assign c_r = c_r_reg;
	assign c_i = c_i_reg;

	assign calc_done = calc_done_reg;
	assign reset_solver = reset_solver_reg;
	assign draw_done_out = draw_done_out_reg;

	always @(posedge clk) begin

		// reset state machine
		if (reset) begin
			state <= 4'd0 ;
				// vga_sram_write <= 1'b0 ; // set to on if a write operation to bus
				// timer <= 0;
		end

		else begin 
			case (state) 

				4'd0: begin // init
					reset_solver_reg <= 1'b1;
					calc_done_reg <= 1'b0;
					draw_done_out_reg <=1'b0;
					vga_x_cood_reg <= i;
					vga_y_cood_reg <= 10'd0;
					c_r_reg =  27'b1110_00000000000000000000000 + (i * 27'b0000_00000001001100110011000); // -2 + i *3/640
					c_i_reg =  27'b0001_00000000000000000000000; // 1

					state[i] <= 4'd1;

				end

				4'd1: begin // wait for solver
					reset_solver_reg <= 1'b0;
					if (~done) begin
						state <= 4'd1;
					end
					else begin 
						state <= 4'd2;

					end

				end

				4'd2: begin // set pixel color, incr solver

					if (iter >= max_iter) begin
						pixel_color_reg <= 8'b_000_000_00 ; // black
					end
					else if (iter >= (max_iter >>> 1)) begin
						pixel_color_reg <= 8'b_011_001_00 ; // white
					end
					else if (iter >= (max_iter >>> 2)) begin
						pixel_color_reg <= 8'b_011_001_00 ;
					end
					else if (iter >= (max_iter >>> 3)) begin
						pixel_color_reg <= 8'b_101_010_01 ;
					end
					else if (iter >= (max_iter >>> 4)) begin
						pixel_color_reg <= 8'b_011_001_01 ;
					end
					else if (iter >= (max_iter >>> 5)) begin
						pixel_color_reg <= 8'b_001_001_01 ;
					end
					else if (iter >= (max_iter >>> 6)) begin
						pixel_color_reg <= 8'b_011_010_10 ;
					end
					else if (iter >= (max_iter >>> 7)) begin
						pixel_color_reg <= 8'b_010_100_10 ;
					end
					else if (iter >= (max_iter >>> 8)) begin
						pixel_color_reg <= 8'b_010_100_10 ;
					end
					else begin
						pixel_color_reg <= 8'b_010_100_10 ;
					end
			
					calc_done_reg <= 1'b1;
					state <= 4'd3;
				end

				4'd3: begin
					// only increment when we've finished writing to the vga 
					if (draw_done) begin 
						calc_done_reg = 1'b0;
						// vga_sram_write <= 1'b1;
						// vga_sram_address <= vga_out_base_address + {22'b0, vga_x_cood} + ({22'b0,vga_y_cood}*640); 
						// vga_sram_writedata <= pixel_color;

						vga_x_cood_reg <= vga_x_cood + 2;
						c_r_reg <= c_r + 27'b0000_00000010011001100110000; // 2 * 3/640 (hardcoded as 6/640)

						if (vga_x_cood_reg > 10'd639) begin
							vga_x_cood_reg <= i;
							c_r_reg <=  27'b1110_00000000000000000000000 + (i * 27'b0000_00000001001100110011000); // -2 + i *3/640

							vga_y_cood_reg <= vga_y_cood + 1;
							c_i_reg <= c_i - 27'b0000_00000001001010011110000; // 2/440

						end 

						// when we reach the end of the vga screen, stop writing!!!
						if (vga_y_cood > 10'd479) begin
							state <= 4'd4;
						end
						// otherwise go back to state 1 and wait for solver to finish!
						else begin
							reset_solver_reg <= 1'b1;
							draw_done_out_reg <= 1'b0;
							state <= 4'd1;
						end
					end
					else begin
						state <= 4'd3;
					end
				end

				4'd4: begin // finished everything!
					calc_done_reg <= 1'b0;
					state <= 4'd4;
					
				end

			endcase

		end




	end




endmodule



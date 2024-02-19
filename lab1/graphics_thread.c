///////////////////////////////////////
/// 640x480 version! 16-bit color
/// This code will segfault the original
/// DE1 computer
/// compile with
/// gcc graphics_thread.c -o th -O2 -lm -pthread
///
///////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <math.h>
#include <pthread.h>

// lock for scanf
pthread_mutex_t scan_lock = PTHREAD_MUTEX_INITIALIZER;

// video display
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define SDRAM_SPAN			  0x04000000
// characters
#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000
/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000 

#define PIO_CLK_BASE   0x00
#define PIO_RESET_BASE 0x10
#define PIO_X_I_BASE   0x20
#define PIO_Y_I_BASE   0x30
#define PIO_Z_I_BASE   0x40
#define PIO_SIGMA_BASE 0x50
#define PIO_BETA_BASE  0x60
#define PIO_RHO_BASE   0x70
#define PIO_X_O_BASE   0x80
#define PIO_Y_O_BASE   0x90
#define PIO_Z_O_BASE   0xA0


// === FPGA ===
volatile unsigned int *pio_clk_addr   = NULL;
volatile unsigned int *pio_reset_addr = NULL;
volatile unsigned int *pio_x_i_addr   = NULL;
volatile unsigned int *pio_y_i_addr   = NULL;
volatile unsigned int *pio_z_i_addr   = NULL;
volatile unsigned int *pio_sigma_addr = NULL;
volatile unsigned int *pio_beta_addr  = NULL;
volatile unsigned int *pio_rho_addr   = NULL;
volatile unsigned int *pio_x_o_addr   = NULL;
volatile unsigned int *pio_y_o_addr   = NULL;
volatile unsigned int *pio_z_o_addr   = NULL;

// graphics primitives functions 
void VGA_text (int, int, char *);
void VGA_text_clear();
void VGA_box (int, int, int, int, short);
void VGA_rect (int, int, int, int, short);
void VGA_line(int, int, int, int, short) ;
void VGA_Vline(int, int, int, short) ;
void VGA_Hline(int, int, int, short) ;
void VGA_disc (int, int, int, short);
void VGA_circle (int, int, int, int);
void VGA_xy(float,float);
void VGA_yz(float,float);
void VGA_xz(float,float);

// 16-bit primary colors
#define red  (0+(0<<5)+(31<<11))
#define dark_red (0+(0<<5)+(15<<11))
#define green (0+(63<<5)+(0<<11))
#define dark_green (0+(31<<5)+(0<<11))
#define blue (31+(0<<5)+(0<<11))
#define dark_blue (15+(0<<5)+(0<<11))
#define yellow (0+(63<<5)+(31<<11))
#define cyan (31+(63<<5)+(0<<11))
#define magenta (31+(0<<5)+(31<<11))
#define black (0x0000)
#define gray (15+(31<<5)+(51<<11))
#define white (0xffff)
int colors[] = {red, dark_red, green, dark_green, blue, dark_blue, 
		yellow, cyan, magenta, gray, black, white};

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
	*(short *)pixel_ptr = (color);\
} while(0)

// MACROS FOR FIXED POINT CONVERSION // 

typedef signed int fix20 ; // 7.20 fixed pt

#define float2fix(a) ((fix20)((a)*1048576.0)) 
#define fix2float(a) ((double)(a)/1048576.0)

#define int2fix(a) ((fix20)(a << 20))
#define fix2int(a) ((int)(a >> 20))

// the light weight buss base
void *h2p_lw_virtual_base;

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// /dev/mem file id
int fd;

// measure time
struct timeval t1, t2;
double elapsedTime;

// drawing varaibles
// ===========================================

/* create a message to be displayed on the VGA 
		and LCD displays */
char text_top_row[40] = "DE1-SoC ARM/FPGA\0";
char text_bottom_row[40] = "Cornell ece5760\0";
char text_next[40] = "Graphics primitives\0";
char text_names[40] = "Eshita, Mattie, Katherine\0";
char num_string[20], time_string[20], out_string[20] ;
char color_index = 0 ;
int color_counter = 0 ;

// position of disk primitive
int disc_x = 0;
// position of circle primitive
int circle_x = 0 ;
// position of box primitive
int box_x = 5 ;
// position of vertical line primitive
int Vline_x = 0;
// position of horizontal line primitive
int Hline_y = 250;

int x_coor = 0;

//  VGA 3D GRAPHING FUNCTIONS // 

void VGA_xy(float x,float y) { 
    VGA_PIXEL((int) (x*4) + 160, (int) (y*4) + 160, cyan );
}
void VGA_yz(float y,float z) { 
    VGA_PIXEL( (int) (y*4) + 420, (int) (z*4) + 60, magenta );
}
void VGA_xz(float x,float z) { 
    VGA_PIXEL( (int) (x*4) + 260, (int) (z*4) + 260, yellow );
}
// Variables to store outputs from the FPGA
fix20 x_o;
fix20 y_o;
fix20 z_o;


///////////////////////////////////////////////////////////////
// THREADS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// temp initial values to store
float temp_x_i = -1;
float temp_y_i = 0.1;
float temp_z_i = 25;
float temp_sigma = 10;
float temp_beta = 8./3.;
float temp_rho = 28;

// change reset value via trigger 
int init_reset = 0; 

// if paused - high
int paused = 0; 
int set = 0;   // what do they wanna change
int change_speed = 0; // change speed or not? 
// min speed 
int speed = 50000;


///////////////////////////////////////////////////////////////
// reset thread  // 
///////////////////////////////////////////////////////////////

void * reset_thread() {

  while (1) {
  
    if (init_reset) {
    
		// update pio pointers w the initial values 
		*(pio_x_i_addr) = float2fix(temp_x_i);
		*(pio_y_i_addr) = float2fix(temp_y_i);
		*(pio_z_i_addr) = float2fix(temp_z_i);
		
		// do the actual reset
		*(pio_clk_addr) = 0;
		*(pio_reset_addr) = 1;
		*(pio_clk_addr) = 1;
		*(pio_clk_addr) = 0;
		*(pio_reset_addr) = 0;
     
      // clear VGA screen 
      VGA_box (0, 0, 639, 479, 0x0000);
      
      // done initializing
      init_reset = 0;
    }
  }
}

///////////////////////////////////////////////////////////////
// draw thread  // 
///////////////////////////////////////////////////////////////

void * draw_thread () { 

	while(1) 
	{
		if (!init_reset) { 

			if (!paused) { 

				*pio_clk_addr = 1; 
				*pio_clk_addr = 0;

				// Read from the FPGA!
				z_o = *pio_z_o_addr;
				y_o = *pio_y_o_addr;
				x_o = *pio_x_o_addr;
				VGA_xy(fix2float(x_o), fix2float(y_o));
				VGA_xz(fix2float(x_o), fix2float(z_o));
				VGA_yz(fix2float(y_o), fix2float(z_o));

				// add printing text here 

				sprintf(out_string, "x and y");
				VGA_text (20, 35, out_string);
				sprintf(out_string, "x and z");
				VGA_text (50, 35, out_string);
				sprintf(out_string, "y and z");
				VGA_text (30, 55, out_string);

				sprintf(out_string, "x init: %f", temp_x_i);
				VGA_text (35, 1, out_string);
				sprintf(out_string, "y init: %f", temp_y_i);
				VGA_text (35, 2, out_string);
				sprintf(out_string, "z init: %f", temp_z_i);
				VGA_text (35, 3, out_string);

				sprintf(out_string, "sigma: %f", temp_sigma);
				VGA_text (55, 1, out_string);
				sprintf(out_string, "beta: %f", temp_beta);
				VGA_text (55, 2, out_string);
				sprintf(out_string, "rho: %f", temp_rho);
				VGA_text (55, 3, out_string);



				usleep(speed);
			}

		}

	} 

}


///////////////////////////////////////////////////////////////
// scan thread  // 
///////////////////////////////////////////////////////////////

void * scan_thread () {

	while (1) { 
		// which category to change

		printf("0: init, 1: pause, 2: speed, 3: params, 4: clear -- ");
    	scanf("%i", &set);

		switch ( set ) {
		
			case 0:  // changing initial values

				printf("Enter x: ");
				scanf("%f", &temp_x_i);
				printf("Enter y: ");
				scanf("%f", &temp_y_i);
				printf("Enter z: ");
				scanf("%f", &temp_z_i);
				init_reset = 1; // reset
			
			break;

			case 1:  // pause or play 

				paused = !paused;
				break;

			case 2: // change the drawing speed 

				printf("Decrease (0) or Increase (1) speed? ");
				scanf("%i", &change_speed);

				if (change_speed) { 
					// need speed cap? 
					speed = speed - 7000;  // less sleep
				}
				else { 
					speed = speed + 7000; // add more sleep 
				}

				break;

			case 3: // change sigma, rho, beta 

				printf("Enter sigma: ");
				scanf("%f", &temp_sigma);
				printf("Enter beta: ");
				scanf("%f", &temp_beta);
				printf("Enter rho: ");
				scanf("%f", &temp_rho);
				*(pio_sigma_addr) = float2fix(temp_sigma);
				*(pio_beta_addr) = float2fix(temp_beta);
				*(pio_rho_addr) = float2fix(temp_rho);
				init_reset=1;
        
        	break;

			case 4: // clear screen 
				VGA_box (0, 0, 639, 479, 0x0000);

			break;

		} 

	} 

} 

///////////////////////////////////////////////////////////////
// main   // 
///////////////////////////////////////////////////////////////

int main(void)
{
  	
	// === need to mmap: =======================
	// FPGA_CHAR_BASE
	// FPGA_ONCHIP_BASE      
	// HW_REGS_BASE        
  
	// === get FPGA addresses ==================
    // Open /dev/mem
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}
    
    // get virtual addr that maps to physical
	h2p_lw_virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );	
	if( h2p_lw_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap1() failed...\n" );
		close( fd );
		return(1);
	}
    
	// === get VGA char addr =====================
	// get virtual addr that maps to physical
	vga_char_virtual_base = mmap( NULL, FPGA_CHAR_SPAN, ( 	PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_CHAR_BASE );	
	if( vga_char_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap2() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA LED control 
	vga_char_ptr =(unsigned int *)(vga_char_virtual_base);

	// === get VGA pixel addr ====================
	// get virtual addr that maps to physical
	vga_pixel_virtual_base = mmap( NULL, SDRAM_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, SDRAM_BASE);	
	if( vga_pixel_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA pixel buffer
	vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

    // PIO POINTER STUFF
    // Maps to FPGA registers
    pio_clk_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CLK_BASE );
    pio_reset_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_BASE );
    pio_x_i_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_X_I_BASE );
    pio_y_i_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_Y_I_BASE );
    pio_z_i_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_Z_I_BASE );
    pio_sigma_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_SIGMA_BASE );
    pio_beta_addr  = (unsigned int *)(h2p_lw_virtual_base +  PIO_BETA_BASE );
    pio_rho_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_RHO_BASE );

    pio_x_o_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_X_O_BASE );
    pio_y_o_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_Y_O_BASE );
    pio_z_o_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_Z_O_BASE );

	/// VISUALIZE ON THE SCREEN /// 

	char text_x[40] = "init x = ";
	char text_y[40] = "init y = ";
	char text_z[40] = "init z = ";
	char text_sig[40] = "sig = ";
	char text_beta[40] = "beta = ";
	char text_rho[40] = "rho = ";
	char text_speed[40] = "usleep = ";
	char text_status[40] = "status = ";


	// clear the screen
	VGA_box (0, 0, 639, 479, 0x0000);
	// clear the text
	VGA_text_clear();
	// write text
	VGA_text (10, 1, text_top_row);
	VGA_text (10, 2, text_bottom_row);
	VGA_text (10, 3, text_next);
	VGA_text (10, 4, text_names);
	
    // Initial values to send to fpga 
    *pio_z_i_addr = int2fix(25);
    *pio_y_i_addr = float2fix(0.1);
    *pio_x_i_addr = int2fix(-1);

    *pio_sigma_addr = int2fix(10);
    *pio_beta_addr = float2fix(8./3.);
    *pio_rho_addr = int2fix(28);

	*(pio_clk_addr) = 0;
	*(pio_reset_addr) = 1;
	*(pio_clk_addr) = 1;
	*(pio_clk_addr) = 0;
	*(pio_reset_addr) = 0;

	// thread identifiers
   	pthread_t thread_scan, thread_draw, thread_reset;

	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
	
	 // now the threads
	pthread_create( &thread_reset, NULL, reset_thread, NULL );
	pthread_create( &thread_draw, NULL, draw_thread, NULL );
	pthread_create( &thread_scan, NULL, scan_thread, NULL );

	pthread_join( thread_reset, NULL );
	pthread_join( thread_draw, NULL );
	pthread_join( thread_scan, NULL );

   	return 0;

} // end main

/****************************************************************************************
 * Subroutine to send a string of text to the VGA monitor 
****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr)
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset;
	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while ( *(text_ptr) )
	{
		// write to the character buffer
		*(character_buffer + offset) = *(text_ptr);	
		++text_ptr;
		++offset;
	}
}

/****************************************************************************************
 * Subroutine to clear text to the VGA monitor 
****************************************************************************************/
void VGA_text_clear()
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset, x, y;
	for (x=0; x<79; x++){
		for (y=0; y<59; y++){
	/* assume that the text string fits on one line */
			offset = (y << 7) + x;
			// write to the character buffer
			*(character_buffer + offset) = ' ';		
		}
	}
}

/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_box(int x1, int y1, int x2, int y2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
	if (x1>x2) SWAP(x1,x2);
	if (y1>y2) SWAP(y1,y2);
	for (row = y1; row <= y2; row++)
		for (col = x1; col <= x2; ++col)
		{
			//640x480
			//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
			// set pixel color
			//*(char *)pixel_ptr = pixel_color;	
			VGA_PIXEL(col,row,pixel_color);	
		}
}


// =============================================
// === Draw a line
// =============================================
//plot a line 
//at x1,y1 to x2,y2 with color 
//Code is from David Rodgers,
//"Procedural Elements of Computer Graphics",1985
void VGA_line(int x1, int y1, int x2, int y2, short c) {
	int e;
	signed int dx,dy,j, temp;
	signed int s1,s2, xchange;
     signed int x,y;
	char *pixel_ptr ;
	
	/* check and fix line coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
        
	x = x1;
	y = y1;
	
	//take absolute value
	if (x2 < x1) {
		dx = x1 - x2;
		s1 = -1;
	}

	else if (x2 == x1) {
		dx = 0;
		s1 = 0;
	}

	else {
		dx = x2 - x1;
		s1 = 1;
	}

	if (y2 < y1) {
		dy = y1 - y2;
		s2 = -1;
	}

	else if (y2 == y1) {
		dy = 0;
		s2 = 0;
	}

	else {
		dy = y2 - y1;
		s2 = 1;
	}

	xchange = 0;   

	if (dy>dx) {
		temp = dx;
		dx = dy;
		dy = temp;
		xchange = 1;
	} 

	e = ((int)dy<<1) - dx;  
	 
	for (j=0; j<=dx; j++) {
		//video_pt(x,y,c); //640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (y<<10)+ x; 
		// set pixel color
		//*(char *)pixel_ptr = c;
		VGA_PIXEL(x,y,c);			
		 
		if (e>=0) {
			if (xchange==1) x = x + s1;
			else y = y + s2;
			e = e - ((int)dx<<1);
		}

		if (xchange==1) y = y + s2;
		else x = x + s1;

		e = e + ((int)dy<<1);
	}
}

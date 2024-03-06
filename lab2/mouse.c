///////////////////////////////////////
/// 640x480 version! 16-bit color
/// This code will segfault the original
/// DE1 computer
/// compile with
/// gcc mandelbrot.c -o md -O2 -lm -pthread
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

// lock for scanf

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
#define HW_REGS_SPAN          0x00005000 



// graphics primitives
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


// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
	*(short *)pixel_ptr = (color);\
} while(0)


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

///////////////////////////////////////////////////////////////
// ZOOM STUFF FOR MANDELBROT 
///////////////////////////////////////////////////////////////
// MACROS FOR FIXED POINT CONVERSION // 
#define PIO_CR_INIT_BASE        0x00001000 
#define PIO_CI_INIT_BASE        0x00001010 
#define PIO_CI_STEP_BASE        0x00001020
#define PIO_CR_STEP_BASE        0x00001030
#define PIO_RESET_FULL_BASE     0x00001040
#define PIO_DONE_DONE_BASE      0x00001050
#define PIO_MAX_ITER_BASE      	0x00001060

typedef signed int fix23 ; // 4.23 fixed pt

#define float2fix(a) ((fix23)((a)*8388608.0)) 
#define fix2float(a) ((double)(a)/8388608.0)

#define int2fix(a) ((fix23)(a << 23))
#define fix2int(a) ((int)(a >> 23))

float c_r_init = -2.0;
float c_i_init = 1.0;
// depending on how many iterators we have this shold change 
float c_r_step = 12.0/640.0;
float c_i_step = 2.0/480.0;
float max_iter = 1000.0;

int done_done = 0;
int set = 0;


/* create a message to be displayed on the VGA 
		and LCD displays */
char text_top_row[40] = "DE1-SoC ARM/FPGA\0";
char text_bottom_row[40] = "Cornell ece5760\0";
char text_next[40] = "Graphics primitives\0";
char num_string[20], time_string[20], out_string[20] ;

char xy_topleft[10], xy_topright[10], xy_bottomleft[10], xy_bottomright[10] ;
char mouse_coord[20];
char new_coordinate[20];
char zoom_amt[20];
char max_iter_str[40];

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


/// PRINT DATA FUNCTION 
void print_data(float c_r_init, float c_i_init, float c_r_step, float c_i_step, int iter) {
  	VGA_text_clear();
 
  	// Top left xy
	sprintf( xy_topleft, "(%4.4f, %4.4f)", c_r_init, c_i_init );
	VGA_text (1, 1, xy_topleft);

	// Top right xy
	sprintf( xy_bottomleft, "(%4.4f, %4.4f)", (c_r_init+(c_r_step*640)), c_i_init );
	VGA_text (58, 1, xy_bottomleft);

	// Bottom right xy
	sprintf( xy_topright, "(%4.4f, %4.4f)", (c_r_init+(c_r_step*640)), (c_i_init+(c_i_step*480)) );
	VGA_text (58, 58, xy_topright);

	// Bottom left xy
	sprintf( xy_bottomleft, "(%4.4f, %4.4f)", c_r_init, (c_i_init+(c_i_step*480)) );
	VGA_text (1, 58, xy_bottomleft);

	// middle
	sprintf( new_coordinate, "Middle xy: (%4.4f, %4.4f)", (c_r_init+(c_r_step*320)), (+(c_i_step*240)) );
	VGA_text (10, 6, new_coordinate);

	// number of max iteartons
	sprintf( max_iter_str, "Max Iter: %d", iter );
	VGA_text (10, 10, max_iter_str);
 
}


///////////////////////////////////////////////////////////////
// main   // 
///////////////////////////////////////////////////////////////


int main(void)
{
  	// === FPGA ===
   	volatile unsigned int *pio_cr_init_addr = NULL;
	volatile unsigned int *pio_ci_init_addr = NULL;
	volatile unsigned int *pio_ci_step_addr  = NULL;
	volatile unsigned int *pio_cr_step_addr  = NULL;
	volatile unsigned int *pio_reset_full_addr  = NULL;
	volatile unsigned int *pio_done_done_addr  = NULL;
	volatile unsigned int *pio_max_iter_addr  = NULL;

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
    
	//  .reset (~KEY[0]),
    // Get the address that maps to the FPGA pixel buffer
	vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

	// ===========================================

	// clear the screen
	VGA_box (0, 0, 639, 479, 0x0000);
	// clear the text
	VGA_text_clear();
	// write text
	VGA_text (10, 1, text_top_row);
	VGA_text (10, 2, text_bottom_row);
	VGA_text (10, 3, text_next);

	double elapsed_time;

	pio_cr_step_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CR_STEP_BASE );
	pio_ci_step_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CI_STEP_BASE );
	pio_ci_init_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CI_INIT_BASE );
	pio_cr_init_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CR_INIT_BASE );
	pio_reset_full_addr= (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_FULL_BASE );
	pio_done_done_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_DONE_DONE_BASE );
	pio_max_iter_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_MAX_ITER_BASE );

	*pio_cr_init_addr = float2fix(-2.0);
	*pio_ci_init_addr = float2fix(1.0);
	*pio_cr_step_addr = float2fix(12.0/640.0); // x incrementer 
	*pio_ci_step_addr = float2fix(2.0/480.0);
	*pio_max_iter_addr = float2fix(1000.0);

    //////// MOUSE STUFF ///////// 

    int fd, bytes;
    unsigned char data[3];

    const char *pDevice = "/dev/input/mice";

    // Open Mouse
    fd = open(pDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pDevice);
        return -1;
    }

    int left, middle, right;// button presses
    signed char x, y, scroll, prev_x, prev_y; // mouse coordinates
	int x_vga = 0;
	int y_vga = 0;
	int x_accum = 0;
	int y_accum = 0;

    //////// MOUSE STUFF /////////

	while(1) 
	{
	
        // Read Mouse     
        bytes = read(fd, data, sizeof(data));

        if(bytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            // middle = data[0] & 0x4;

            x = data[1];
            y = data[2];

			x_accum += x/2;
			y_accum += y/2;

			// calculate middle coordinates 
			float x_temp = c_r_init + x_accum*c_r_step + c_r_step*320;
        	float y_temp = c_i_init + y_accum*c_i_step + c_i_step*240;

			sprintf( new_coordinate, "Middle xy: (%4.4f, %4.4f)", x_temp, y_temp );
        	VGA_text (10, 6, new_coordinate);
			// end middle coordinate calculation

			// if the left mouse button was pressed - reduce cr step and ci step by half
			if (left == 1 ) { 
				c_r_step = c_r_step/2;
				c_i_step = c_i_step/2;
				*pio_cr_step_addr = float2fix(c_r_step);
				*pio_ci_step_addr = float2fix(c_i_step);

				// do we need to reset here???? 

				print_data(c_r_init, c_i_init, c_r_step, c_i_step, *(pio_max_iter_addr));
				x_accum = 0;
				y_accum = 0;

			}

			else if (right == 2 ) {
				c_r_step = c_r_step*2;
				c_i_step = c_i_step*2;
				*pio_cr_step_addr = float2fix(c_r_step);
				*pio_ci_step_addr = float2fix(c_i_step);
				// do we need to reset here???? 
				print_data(c_r_init, c_i_init, c_r_step, c_i_step, *(pio_max_iter_addr));
				x_accum = 0;
				y_accum = 0;

			}

			else if (middle == 4) {

				c_r_init = c_r_init + x_accum*c_r_step;
				c_i_init = c_i_init + y_accum*c_i_step;
				*pio_cr_init_addr = float2fix(c_r_init);
				*pio_ci_init_addr = float2fix(c_i_init);

				// do we need to reset ?? idk 

				print_data(c_r_init, c_i_init, c_r_step, c_i_step, *(pio_max_iter_addr));
				x_accum = 0;
				y_accum = 0;
			}

			
            printf("x=%d, y=%d, left=%d, middle=%d, right=%d\n", x, y, left, middle, right);

        }

		*pio_cr_init_addr = float2fix(c_r_init);
		*pio_ci_init_addr = float2fix(c_i_init);
		*pio_cr_step_addr = float2fix(c_r_step);
		*pio_ci_step_addr = float2fix(c_i_step);
		*pio_max_iter_addr = float2fix(max_iter);

		*pio_reset_full_addr = 1;
		*pio_reset_full_addr = 0;

		// time_t start_time = time(NULL);
		// start timer
		gettimeofday(&t1, NULL);

		while(!done_done){
			done_done = *pio_done_done_addr;
		}

		gettimeofday(&t2, NULL);
		elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000000.0;      // sec to us
		elapsed_time += (t2.tv_usec - t1.tv_usec) ;   // us 
        printf("time: %.2f ", elapsed_time);

		usleep(10000);
		
	} // end while(1)

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

/****************************************************************************************
 * Draw a outline rectangle on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_rect(int x1, int y1, int x2, int y2, short pixel_color)
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
	// left edge
	col = x1;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
		
	// right edge
	col = x2;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
	
	// top edge
	row = y1;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);
	}
	
	// bottom edge
	row = y2;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);
	}
}

/****************************************************************************************
 * Draw a horixontal line on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_Hline(int x1, int y1, int x2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (x1>x2) SWAP(x1,x2);
	// line
	row = y1;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
}

/****************************************************************************************
 * Draw a vertical line on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_Vline(int x1, int y1, int y2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (y2<0) y2 = 0;
	if (y1>y2) SWAP(y1,y2);
	// line
	col = x1;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);			
	}
}


/****************************************************************************************
 * Draw a filled circle on the VGA monitor 
****************************************************************************************/

void VGA_disc(int x, int y, int r, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col, rsqr, xc, yc;
	
	rsqr = r*r;
	
	for (yc = -r; yc <= r; yc++)
		for (xc = -r; xc <= r; xc++)
		{
			col = xc;
			row = yc;
			// add the r to make the edge smoother
			if(col*col+row*row <= rsqr+r){
				col += x; // add the center point
				row += y; // add the center point
				//check for valid 640x480
				if (col>639) col = 639;
				if (row>479) row = 479;
				if (col<0) col = 0;
				if (row<0) row = 0;
				//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
				// set pixel color
				//*(char *)pixel_ptr = pixel_color;
				VGA_PIXEL(col,row,pixel_color);	
			}
					
		}
}

/****************************************************************************************
 * Draw a  circle on the VGA monitor 
****************************************************************************************/

void VGA_circle(int x, int y, int r, int pixel_color)
{
	char  *pixel_ptr ; 
	int row, col, rsqr, xc, yc;
	int col1, row1;
	rsqr = r*r;
	
	for (yc = -r; yc <= r; yc++){
		//row = yc;
		col1 = (int)sqrt((float)(rsqr + r - yc*yc));
		// right edge
		col = col1 + x; // add the center point
		row = yc + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
		// left edge
		col = -col1 + x; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
	}
	for (xc = -r; xc <= r; xc++){
		//row = yc;
		row1 = (int)sqrt((float)(rsqr + r - xc*xc));
		// right edge
		col = xc + x; // add the center point
		row = row1 + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
		// left edge
		row = -row1 + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
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
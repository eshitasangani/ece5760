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
#include <pthread.h>

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
#define PIO_KEY0_BASE 			0x00001070

typedef signed int fix23 ; // 4.23 fixed pt

#define float2fix(a) ((fix23)((a)*8388608.0)) 
#define fix2float(a) ((double)(a)/8388608.0)

#define int2fix(a) ((fix23)(a << 23))
#define fix2int(a) ((int)(a >> 23))

float c_r_init = -2.0;
float c_i_init = 1.0;
float c_r_step = 3.0/640.0;
float c_i_step = 2.0/480.0;
int max_iter = 1000;

int done_done = 0;
int set = 0;

///////////////////////////////////////////////////////////////
// main   // 
///////////////////////////////////////////////////////////////
double elapsed_time;
volatile unsigned int *pio_cr_init_addr = NULL;
volatile unsigned int *pio_ci_init_addr = NULL;
volatile unsigned int *pio_ci_step_addr  = NULL;
volatile unsigned int *pio_cr_step_addr  = NULL;
volatile unsigned int *pio_reset_full_addr  = NULL;
volatile unsigned int *pio_done_done_addr  = NULL;
volatile unsigned int *pio_max_iter_addr  = NULL;
volatile unsigned int *pio_key0_addr  = NULL;

void print_stats(){

	gettimeofday(&t1, NULL);

	while(!*pio_done_done_addr){}

	gettimeofday(&t2, NULL);

	elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000000.0;      // sec to us
	elapsed_time += (t2.tv_usec - t1.tv_usec) ;   // us 
	elapsed_time = elapsed_time * 0.001;
	printf("\ntime: %.2f ms ", elapsed_time);

	printf("\nMax Iterations: %d\n", *pio_max_iter_addr);

	printf("X: %f, %f\n", fix2float(*pio_cr_init_addr), (fix2float(*pio_cr_init_addr) + 640 * fix2float(*pio_cr_step_addr)));
	printf("Y: %f, %f\n", fix2float(*pio_ci_init_addr), (fix2float(*pio_ci_init_addr) + 480 * fix2float(*pio_ci_step_addr)));
}

///////////////////////////////////////////////////////////////
// KEY0 RESET thread  // 
///////////////////////////////////////////////////////////////
void * reset_thread() {

    while (1) {
        if ( *pio_key0_addr == 1 ) {

            // reset pio initial/fixed conditions
            *pio_cr_init_addr = float2fix(-2.0);
			*pio_ci_init_addr = float2fix(1.0);
			*pio_cr_step_addr = float2fix(3.0/640.0);
			*pio_ci_step_addr = float2fix(2.0/480.0);
			*pio_max_iter_addr = (1000);

            *pio_reset_full_addr = 1;
			*pio_reset_full_addr = 0;
        }
    }
}

///////////////////////////////////////////////////////////////
// scan thread  // 
///////////////////////////////////////////////////////////////
void * scan_thread () { 

	while (1) { 
		printf("1: cr init, 2: ci init, 3: cr step, 4: ci step, 5:max iter \n");
		scanf("%i", &set);

		switch (set) {
			case 1: 
				printf("enter cr init: ");
				scanf("%f", &c_r_init);
				*pio_cr_init_addr = float2fix(c_r_init);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
			case 2:
				printf("enter ci init: ");
				scanf("%f", &c_i_init);
				*pio_ci_init_addr = float2fix(c_i_init);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
			case 3: 
				printf("enter cr step: ");
				scanf("%f", &c_r_step);
				*pio_cr_step_addr = float2fix(c_r_step);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
			case 4: 
				printf("enter ci step: ");
				scanf("%f", &c_i_step);
				*pio_ci_step_addr = float2fix(c_i_step);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
			case 5: 
				printf("max iter: ");
				scanf("%d", &max_iter);
				*pio_max_iter_addr = (max_iter);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
		}

	}
}


///////////////////////////////////////////////////////////////
// update_max_iter_thread  // 
///////////////////////////////////////////////////////////////

void * update_max_iter_thread () { 

	while (1) { 

		printf(":max iter \n");
		scanf("%i", &set);

		switch (set) {
			case 1: 
				printf("max iter: ");
				scanf("%d", &max_iter);

				*pio_max_iter_addr = (max_iter);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				break;
		} 
		
		print_stats();
	}
}

///////////////////////////////////////////////////////////////
// serial mouse read thread  // 
///////////////////////////////////////////////////////////////

void *read_mouse_thread() { 

	//////// BEGIN  INIT ///////// 

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
	float x_center = -0.5;
	float y_center = 0.;

    //////// END MOUSE INIT /////////

	while (1) { 

		// Read Mouse     
        bytes = read(fd, data, sizeof(data));

		if(bytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            middle = data[0] & 0x4;

            x = data[1];
            y = data[2];

			x_accum += x/2;
			y_accum += y/2;

			float x_temp = c_r_init + x_accum*c_r_step + c_r_step*320;
        	float y_temp = c_i_init + y_accum*c_i_step + c_i_step*240;

			// if the left mouse button was pressed - reduce cr step and ci step by half
			if (left == 1 ) { 
				c_r_step = c_r_step/2;
				c_i_step = c_i_step/2;

				*pio_cr_step_addr = float2fix(c_r_step);
				*pio_ci_step_addr = float2fix(c_i_step);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;

				print_stats();
				x_accum = 0;
				y_accum = 0;

			}

			else if (right == 2 ) {
				c_r_step = c_r_step*2;
				c_i_step = c_i_step*2;
				*pio_cr_step_addr = float2fix(c_r_step);
				*pio_ci_step_addr = float2fix(c_i_step);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;
				print_stats();


			}

			else if (middle == 4) {

				c_r_init = c_r_init + x_accum*c_r_step;
				c_i_init = c_i_init + y_accum*c_i_step;
				*pio_cr_init_addr = float2fix(c_r_init);
				*pio_ci_init_addr = float2fix(c_i_init);
				*pio_reset_full_addr = 1;
				*pio_reset_full_addr = 0;

				print_stats();

			}
			
            // printf("x=%d, y=%d, left=%d, middle=%d, right=%d\n", x, y, left, middle, right);

        }

	}

}

int main(void)
{
  	// === FPGA ===
   	
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
    
	pio_cr_step_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CR_STEP_BASE );
	pio_ci_step_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CI_STEP_BASE );
	pio_ci_init_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CI_INIT_BASE );
	pio_cr_init_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_CR_INIT_BASE );
	pio_reset_full_addr= (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_FULL_BASE );
	pio_done_done_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_DONE_DONE_BASE );
	pio_max_iter_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_MAX_ITER_BASE );
	pio_key0_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_KEY0_BASE );

	/// SEND INITIAL CONDITIONS /// 

	*pio_cr_init_addr = float2fix(-2.0);
	*pio_ci_init_addr = float2fix(1.0);
	*pio_cr_step_addr = float2fix(3.0/640.0);
	*pio_ci_step_addr = float2fix(2.0/480.0);
	*pio_max_iter_addr = (1000.0);

	*pio_reset_full_addr = 1;
	*pio_reset_full_addr = 0;

	print_stats();

	// thread identifiers
   	pthread_t thread_scan, thread_reset, thread_read_mouse, thread_update_max_iter;

	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

	// now the threads
	pthread_create( &thread_reset, NULL, reset_thread, NULL );
	pthread_create( &thread_scan, NULL, scan_thread, NULL );
	pthread_create( &thread_read_mouse, NULL, read_mouse_thread, NULL );
	pthread_create( &thread_update_max_iter, NULL, update_max_iter_thread, NULL );

	pthread_join( thread_reset, NULL );
	pthread_join( thread_scan, NULL );
	pthread_join( thread_read_mouse, NULL );
	pthread_join( thread_update_max_iter, NULL );

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

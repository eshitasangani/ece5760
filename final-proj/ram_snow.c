///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc ram_snow.c -o snow -pthread
///////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <pthread.h>
#include <math.h>
#include "address_map_arm_brl4.h"

/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000  // WHAT WE AHD B4
// #define HW_REGS_SPAN          0x10000000 

#define FPGA_SDRAM_BASE      0xC0000000
//#define FPGA_ONCHIP_END       0xC803FFFF
// modified for 640x480
// #define FPGA_ONCHIP_SPAN      0x00040000
#define FPGA_SDRAM_SPAN      0x04000000

#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000

// SRAM FOR SYNCRHONIZXATIOBN!! 

// h2f_axi_master; scratch RAM at 0xC0000000
// issue from last time 
#define FPGA_SRAM_BASE      0xC8000000
#define FPGA_SRAM_SPAN      0x00002000


//// BASE ADDRESSES FOR PIO ADDRESSES ////
#define PIO_ALPHA_BASE      0x100
#define PIO_BETA_BASE       0x110
#define PIO_GAMMA_BASE      0x120

#define PIO_RESET_FROM_BASE 0x130

#define PIO_IS_FROZEN_BASE  0x140
#define PIO_FROZEN_Y_BASE   0x150

#define PIO_RESET_TO_BASE   0x160
#define PIO_DONE_SEND_BASE  0x170

#define PIO_VALID_TO_HPS_BASE  0x180
#define PIO_ONE_ITER_DONE_BASE  0x190

// =============================== FPGA ============================
volatile unsigned int *pio_alpha_addr       = NULL;
volatile unsigned int *pio_beta_addr        = NULL;
volatile unsigned int *pio_gamma_addr       = NULL;

volatile unsigned int *pio_reset_addr       = NULL;
volatile unsigned int *pio_reset_to_addr    = NULL;
volatile unsigned int *pio_is_frozen_addr   = NULL;
volatile unsigned int *pio_frozen_y_addr    = NULL;
volatile unsigned int *pio_done_send_addr   = NULL;

// Variables to store outputs from the FPGA
typedef struct {
    // fix18 frozen_x;
    int is_frozen;
    int frozen_y;
} yCoordinate;

#define BUFFER_SIZE 11
yCoordinate buffer[BUFFER_SIZE];
int buffer_index = 0;


/* function prototypes */
void VGA_text (int, int, char *);
void VGA_text_clear();
void VGA_box (int, int, int, int, short);
void VGA_cell (int, int, int, int, short);
void VGA_rect (int, int, int, int, short);

// MACROS FOR FIXED POINT CONVERSION // 
typedef signed int fix18 ;
#define mult2fix18(a,b) ((fix18)(((( signed long long)(a))*(( signed long long)(b)))>>18)) 
#define float2fix18(a) ((fix18)((a)*262144.0f)) // 2^18
#define fix2float18(a) ((float)(a)/262144.0f) 
#define int2fix28(a) ((a)<<18);

// pixel macro
// 16 bit color
// pixel macro -- shift-left in the pixel pointer is specified in the Video Core Manual
// probably becuase the DMA addressing is all in bytes
#define VGA_PIXEL(x,y,color) do{\
int *pixel_ptr ;\
pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
*(short *)pixel_ptr = (color);\
} while(0)

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

// the light weight buss base
void *h2p_lw_virtual_base;

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// BASE FOR SRAM 
void *h2p_virtual_base;
volatile unsigned int * sram_ptr = NULL ;

// /dev/mem file id
int fd;


// shared memory 
key_t mem_key=0xf0;
int shared_mem_id; 
int *shared_ptr;
int shared_time;
int shared_note;
char shared_str[64];

int y_is_frozen;
int y_frozen_coor;

///////////////////////////////////////////////////////////////
// THREADS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// INITAL VARIABLES TO SEND TO FGPA 
float temp_alpha = 1.0;
float temp_beta = 0.8;
float temp_gamma = 0.01;

// change reset value via trigger 
int init_reset = 0; 
int set = 0;
short color;

///////////////////////////////////////////////////////////////
// reset thread  // 
///////////////////////////////////////////////////////////////

void * reset_thread() {

  while (1) {
  
    if (*pio_reset_to_addr) {
    
    // update pio pointers w the initial values 
    *(pio_alpha_addr) = float2fix18(temp_alpha);
    *(pio_beta_addr)  = float2fix18(temp_beta);
    *(pio_gamma_addr) = float2fix18(temp_gamma);

    // clear VGA screen 
    VGA_box (0, 0, 639, 479, 0x0000);

    }
  }
}

///////////////////////////////////////////////////////////////
// scan thread  // 
///////////////////////////////////////////////////////////////
void * scan_thread () { 

    while (1) { 
        printf("1: alpha, 2: beta 3. gamma \n");
        scanf("%i", &set);

        switch (set) {
            case 1: 
                printf("enter alpha: ");
                scanf("%f", &temp_alpha);
                *pio_alpha_addr = float2fix18(temp_alpha);
                break;

            case 2: 
                printf("enter beta: ");
                scanf("%f", &temp_beta);
                *pio_beta_addr = float2fix18(temp_beta);
                break;

            case 3: 
                printf("enter gamma: ");
                scanf("%f", &temp_gamma);
                *pio_gamma_addr = float2fix18(temp_gamma);
                break;
        }
    }
}


/////////////////////////////////////////////////////////////
// frozen thread  // 
/////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// draw thread  // 
///////////////////////////////////////////////////////////////


int main(void)
{

    // Declare volatile pointers to I/O registers (volatile     // means that IO load and store instructions will be used   // to access these pointer locations, 
    // instead of regular memory loads and stores) 

    // === shared memory =======================
    // with video process
    shared_mem_id = shmget(mem_key, 100, IPC_CREAT | 0666);
    //shared_mem_id = shmget(mem_key, 100, 0666);
    shared_ptr = shmat(shared_mem_id, NULL, 0);

    // === need to mmap: =======================
    // FPGA_CHAR_BASE
    // FPGA_ONCHIP_BASE      
    // HW_REGS_BASE        
  
    // === get FPGA addresses ==================
    // Open /dev/mem
    if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 )    {
        printf( "ERROR: could not open \"/dev/mem\"...\n" );
        return( 1 );
    }
    
    // get virtual addr that maps to physical
    // this includes the new sram that we have jus added 
    h2p_lw_virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE ); 
    if( h2p_lw_virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap1() failed...\n" );
        close( fd );
        return(1);
    }

    // non lightweight bus 
    //  RAM FPGA parameter/analyzer addrs 
	h2p_virtual_base = mmap( NULL, FPGA_SRAM_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_SRAM_BASE); 	
	if( h2p_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);
	}

    // === get VGA char addr =====================
    // get virtual addr that maps to physical
    vga_char_virtual_base = mmap( NULL, FPGA_CHAR_SPAN, (   PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_CHAR_BASE ); 
    if( vga_char_virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap2() failed...\n" );
        close( fd );
        return(1);
    }
    
    // Get the address that maps to the FPGA LED control 
    vga_char_ptr =(unsigned int *)(vga_char_virtual_base);


    // === get VGA pixel addr ====================
    // get virtual addr that maps to physical
    vga_pixel_virtual_base = mmap( NULL, FPGA_SDRAM_SPAN, (    PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_SDRAM_BASE);  
    if( vga_pixel_virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap3() failed...\n" );
        close( fd );
        return(1);
    }
    
    // Get the address that maps to the FPGA pixel buffer
    vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

    // ===========================================

    // PIO POINTER STUFF
    // Maps to FPGA registers
    pio_alpha_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_ALPHA_BASE );
    pio_beta_addr    = (unsigned int *)(h2p_lw_virtual_base +  PIO_BETA_BASE );
    pio_gamma_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_GAMMA_BASE );
    pio_is_frozen_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_IS_FROZEN_BASE );
    pio_frozen_y_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_FROZEN_Y_BASE );
    pio_reset_to_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_TO_BASE );
    pio_done_send_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_DONE_SEND_BASE );
    // GET THE ADDRESS THAT MAPS TO THE RAM BUFFERS 
    sram_ptr = (unsigned int *)(h2p_virtual_base);

    /// VISUALIZE ON THE SCREEN /// 

    char text_x[40] = "init alpha = ";
    char text_y[40] = "init beta = ";
    char text_z[40] = "init gamma = ";
    /* create a message to be displayed on the VGA 
          and LCD displays */
    char text_top_row[40]    = "DE1-SoC ARM/FPGA\0";
    char text_bottom_row[40] = "Cornell ece5760\0";

    VGA_box(0,0,639,479,black);
    

    VGA_text (34, 1, text_top_row);
    VGA_text (34, 2, text_bottom_row);
    // VGA_text (34, 3, text_x);
    // VGA_text (34, 4, text_y);
    // VGA_text (34, 5, text_z);

    // Initial values to send to fpga 
    *pio_alpha_addr = float2fix18(temp_alpha);
    *pio_beta_addr  = float2fix18(temp_beta);
    *pio_gamma_addr = float2fix18(temp_gamma);
    
    int i; // loop identifier
    while (1) { 
        if(*pio_reset_to_addr){
            // update pio pointers w the initial values 
            *(pio_alpha_addr) = float2fix18(temp_alpha);
            *(pio_beta_addr)  = float2fix18(temp_beta);
            *(pio_gamma_addr) = float2fix18(temp_gamma);
            *(sram_ptr) = 1;
            *(sram_ptr+1) = 0;
        }
        else{
            // if the fpga signals that we are done with one iteration
            if(*(sram_ptr+1) == 1){
                // every cell in column has a value 
                // draw the whole iteration here 
                
                printf("ONE ITER DONE \n");
                
                for (i = 0; i < 11; i++) { 

                    printf("buffer[%d]: %d; ", i, buffer[i].is_frozen);
                    printf("%d \n", buffer[i].frozen_y);

                    if (buffer[i].is_frozen == 1) { 
                        VGA_box(100, 10 * buffer[i].frozen_y + 100, 
                                        100 + 10, 10 * buffer[i].frozen_y + 110, blue);

                    }
                    else { 
                        VGA_box(100, 10 * buffer[i].frozen_y + 100, 
                                    100 + 10, 10 * buffer[i].frozen_y + 110, white);
                    }
                }
                usleep(15000);
                buffer_index = 0;
                *(sram_ptr+1) = 0;
        
            }

            else {
                // wait for the FPGA value to be valid
                while(*(sram_ptr) == 1) {
                }; // 0th bit = synch signal

                // read values from the FPGA & add to the buffer
                yCoordinate coord; 

                coord.is_frozen = *pio_is_frozen_addr;
                coord.frozen_y  = *pio_frozen_y_addr; 
                printf("is frozen: %d; ", coord.is_frozen);
                printf("frozen y: %d \n", coord.frozen_y);

                buffer[buffer_index] = coord; 
                buffer_index++;

                *(sram_ptr) = 1; // tell the FPGA we are ready for the next value
            }
        }
        
    }

    return 0;

    // VGA_text (10, 1, text_top_row);
    // VGA_text (10, 2, text_bottom_row);
    
} // end main


void VGA_cell(int x1, int y1, int x2, int y2, short pixel_color)
{
    VGA_PIXEL(x1,y1,pixel_color);
    VGA_PIXEL(x1,y2,pixel_color);
    VGA_PIXEL(x2,y1,pixel_color);
    VGA_PIXEL(x2,y2,pixel_color);

}
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
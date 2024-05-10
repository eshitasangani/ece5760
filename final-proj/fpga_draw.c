///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc fpga_draw.c -o draw -pthread
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
#include "address_map_arm_brl4.h"
#include <pthread.h>

// lock for scanf
pthread_mutex_t scan_lock = PTHREAD_MUTEX_INITIALIZER;

/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000                                      

#define FPGA_ONCHIP_BASE      0xC8000000
//#define FPGA_ONCHIP_END       0xC803FFFF
// modified for 640x480
// #define FPGA_ONCHIP_SPAN      0x00040000
#define FPGA_ONCHIP_SPAN      0x00080000

#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000

/* function prototypes */
void VGA_text (int, int, char *);
void VGA_text_clear();
void VGA_box (int, int, int, int, short);
void VGA_line(int, int, int, int, short) ;
void VGA_disc (int, int, int, short);
void VGA_cell (int, int, int, int, short);
// fixed pt
typedef signed int fix28 ;
//multiply two fixed 4:28
#define multfix28(a,b) ((fix28)(((( signed long long)(a))*(( signed long long)(b)))>>28)) 
#define float2fix28(a) ((fix28)((a)*268435456.0f)) // 2^28
#define fix2float28(a) ((float)(a)/268435456.0f) 
#define int2fix28(a) ((a)<<28);
// the fixed point value 4
#define FOURfix28 0x40000000 
#define SIXTEENTHfix28 0x01000000
#define ONEfix28 0x10000000

// the light weight buss base
void *h2p_lw_virtual_base;

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// RAM FPGA command buffer
volatile unsigned int * sram_ptr = NULL ;
void *sram_virtual_base;

// /dev/mem file id
int fd;

// shared memory 
key_t mem_key=0xf0;
int shared_mem_id; 
int *shared_ptr;
int shared_time;
int shared_note;
char shared_str[64];

// loop identifiers
int i,j,k,x,y;


// ----- PIO ADDRESS BASES ----- //
#define PIO_RESET_TO_HPS_BASE       0x00000000 

volatile unsigned int *pio_reset_to_hps = NULL;

///////////////////////////////////////////////// 
#define WIDTH 301
#define HEIGHT 301 

// #define ALPHA 1.0
// #define BETA 0.8
// #define GAMMA 0.01

#define NUM_NEIGHBORS 6

typedef struct {
    float u;   // water used in diffusion
    float v;   // water not used in diffusion
    float s;   // total water
	float next_u; // water used next step 
	float next_v; // water not used next step 
    bool is_receptive;
} Cell;

Cell cells[WIDTH][HEIGHT];
float s_vals[WIDTH][HEIGHT]; // Array to store s values for visualization or debugging
Cell* neighbors[NUM_NEIGHBORS];
Cell frozen[WIDTH][HEIGHT];
int num_neighbors;
float u_avg = 0.0;
float sum_u = 0.0;
short color; 
// 8-bit color
#define rgb(r,g,b) ((((r)&7)<<5) | (((g)&7)<<2) | (((b)&3)))

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	char  *pixel_ptr ;\
	pixel_ptr = (char *)vga_pixel_ptr + ((y)<<10) + (x) ;\
	*(char *)pixel_ptr = (color);\
} while(0)


// Get neighbors for a specific coordinate
int get_num_neighbors(Cell* neighbors[], int x, int y) {
    int count = 0;

	if (y != 0) { // top neighbor, does not rely on if the column is even or odd
		neighbors[count++] = &cells[x][y-1];
	}
	if (y != HEIGHT-1) { // bottom  neighbor, does not rely on if the column is even or odd
		neighbors[count++] = &cells[x][y+1];
	}
	
	if (x % 2 == 0) { // even columns
		if (x != 0) { // left side neighbors, the 0th column does not have left side neighbors
			neighbors[count++] = &cells[x-1][y];
			if (y != 0) { // only top left if y not 0
				neighbors[count++] = &cells[x-1][y-1];
			}
		}
		if (x != WIDTH-1) { // right side neighbors
			neighbors[count++] = &cells[x+1][y];
			if (y != 0) { // only top right if y not 0
				neighbors[count++] = &cells[x+1][y-1];
			}
		}
		
	}
	if (x % 2 == 1){ // odd columns
		// odd numbered columns always have left side neighbors
		neighbors[count++] = &cells[x-1][y];
		if (y != HEIGHT-1) { // only bottom left if y not at bottom
			neighbors[count++] = &cells[x-1][y+1];
		}

		if (x != WIDTH-1) { // right side neighbors
			neighbors[count++] = &cells[x+1][y];
			if (y != HEIGHT-1) { // only bottom right if y not height-1
				neighbors[count++] = &cells[x+1][y+1];
			}
		}
	}

    return count;
}

void initialize_grid() {
    for ( i = 0; i < WIDTH; i++) {
        for ( j = 0; j < HEIGHT; j++) {
            cells[i][j].s = init_beta;
            cells[i][j].is_receptive = false;
            cells[i][j].u = 0;
			cells[i][j].next_u = 0;
			cells[i][j].next_v = 0;
            cells[i][j].v = 0;
        }
    }
    // Set the center cell
    cells[ (WIDTH - 1 ) / 2 ] [(HEIGHT - 1 ) / 2].s = 1.0;
    cells[ (WIDTH - 1 ) / 2 ] [ (HEIGHT - 1 ) / 2].is_receptive = true;

    cells[10][10].s = 1.0;
    cells[10][10].is_receptive = true;
    cells[200][10].s = 1.0;
    cells[200][10].is_receptive = true;
    cells[200][300].s = 1.0;
    cells[200][300].is_receptive = true;
    cells[10][200].s = 1.0;
    cells[10][200].is_receptive = true;

}

///////////////////////////////////////////////////////////////
// THREADS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
float init_alpha = 0.1;
float init_beta = 0.8;
float init_gamma = 0.1;
int set; 
int paused = 0; 
///////////////////////////////////////////////////////////////
// reset thread  // 
///////////////////////////////////////////////////////////////

void * reset_thread() {

	while (1) {
		if(*pio_reset_to_hps)
		{ 
			VGA_box (0, 0, 639, 479, 0x0000);
		}

	}

}

///////////////////////////////////////////////////////////////
// modify paramters through scan thread // 
///////////////////////////////////////////////////////////////

void * scan_thread () {

	while (1) { 
		// which category to change

		printf("0: alpha, 1: beta, 2: gamma, 3: pause, 4: clear -- ");
    	scanf("%i", &set);

		switch ( set ) {
		
			case 0:  // changing alpha 

				printf("Enter alpha: ");
				scanf("%f", init_alpha);
			
			break;

			case 1:  // changing beta 

				printf("Enter beta: ");
				scanf("%f", init_beta);
				
			break; 

			case 2:  // changing gamma 

				printf("Enter gamma: ");
				scanf("%f", init_gamma);

			break; 

			case 3:  // setting paused 

				printf("Pause/Play: ");
				scanf("%f", paused);

			case 4: // clear screen 
				VGA_box (0, 0, 639, 479, 0x0000);

			break;

		} 

	} 

} 

void one_iter() {

    // step 1: determine receptive sites
    for ( i = 0; i < WIDTH; i++) {
        for ( j = 0; j < HEIGHT; j++) {

            if (cells[i][j].is_receptive) {
                cells[i][j].u = 0;
                cells[i][j].v = cells[i][j].s;
                cells[i][j].next_v = cells[i][j].s + init_gamma;
            } 
			else {
                cells[i][j].u = cells[i][j].s;
                cells[i][j].v = 0;
            }
        }
    }

    // step 2: modify cell values == doing actual diffusion
	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {

			if (i == 0 || i == WIDTH-1 || j == 0 || j == HEIGHT-1) { // for edge cells
				cells[i][j].u = init_beta;
				cells[i][j].v = 0;
			}

			else { // everything else
				num_neighbors = get_num_neighbors(neighbors, i, j); // get num neighbors @ specific coord + modify neighbors

				sum_u = 0;

				for (k = 0; k < num_neighbors; k++) {
					sum_u += neighbors[k]->u; // Sum u values of all neighbors
				}

				u_avg = sum_u / num_neighbors; // Calculate average u
				
				cells[i][j].next_u = cells[i][j].u + (init_alpha / 2 * (u_avg - cells[i][j].u)); // update u based on diffusion eq 
				cells[i][j].s = cells[i][j].next_u + cells[i][j].next_v; 

				// Update receptiveness based on the new sp
				if (cells[i][j].s >= 1) {
					cells[i][j].is_receptive = true;
				}

			}

		}
	}

	// step 3: all s values must be updated to determine new boundary 
	for (i = 0; i < WIDTH; i++) { 
		for (j = 0; j < HEIGHT; j++) { 
			if (!cells[i][j].is_receptive) {
				num_neighbors = get_num_neighbors(neighbors, i, j); 

				for (k = 0; k < num_neighbors; k++) {
					if (neighbors[k]->s >= 1) { 

						cells[i][j].is_receptive = true; // one of the neighbors is frozen, so we mark this cell as receptive
					} 
				}

			}

		}
	}

 }

///////////////////////////////////////////////////////////////
// draw thread // 
///////////////////////////////////////////////////////////////

void * draw_thread () {
	initialize_grid();

	for (x = 0; x < 145; x++) { 

		if (!paused) {  // if paused is 0 = means we can continue calculating and drawing 

			one_iter(); // does the actual calculation 
			for (i = 0; i < WIDTH; i++) {
				for (j = 0; j < HEIGHT; j++) {
					int count = 0;

					if (i % 2 == 0) { // even columns 
						if (cells[i][j].s >= 1) { 
							for (x = 0; x < 2; x++) {
								for (y = 0; y < 2; y++) {

									int cellx = (2*i)+x;
									int celly = (2*j)+y;
									VGA_disc(cellx, celly, 0, 0xffff);

								}
							}
						}

					}
					else { // odd columns
						if (cells[i][j].s >= 1) { 
							for (x = 0; x < 2; x++) {
								for (y = 0; y < 2; y++) {
									int cellx = (2*i)+x;
									int celly = (2*j)+y+1;
									VGA_disc(cellx, celly, 0, 0xffff);

								}
								
							}
						}

					}
					
				}
        	}

		}
        
    }

}



int main(void)
{
	// Declare volatile pointers to I/O registers (volatile 	// means that IO load and store instructions will be used 	// to access these pointer locations, 
	// instead of regular memory loads and stores) 

	// === shared memory =======================
	// with video process
	// shared_mem_id = shmget(mem_key, 100, IPC_CREAT | 0666);
 	// //shared_mem_id = shmget(mem_key, 100, 0666);
	// shared_ptr = shmat(shared_mem_id, NULL, 0);

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
	vga_pixel_virtual_base = mmap( NULL, FPGA_ONCHIP_SPAN, ( 	PROT_READ | PROT_WRITE ), MAP_SHARED, fd, 			FPGA_ONCHIP_BASE);	
	if( vga_pixel_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA pixel buffer
	vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

	// RAM FPGA PARAMETER ADDING 
	sram_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_ONCHIP_BASE);

	if (sram_virtual_base == MAP_FAILED) { 
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);

	}
	
	sram_ptr = (unsigned int *) (sram_virtual_base);
	pio_reset_to_hps = (unsigned int *)(h2p_lw_virtual_base + PIO_RESET_TO_HPS_BASE);

	// ===========================================

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
            VGA_disc(col, row, 0, pixel_color);
        }
}

/****************************************************************************************
 * Draw a filled circle on the VGA monitor using GPU FSM
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

                // set up scratch pad parameters
                *(sram_ptr+1) = col-0.5;
                *(sram_ptr+2) = row-0.5;
                *(sram_ptr+3) = col;
                *(sram_ptr+4) = row;
                *(sram_ptr+5) = pixel_color;
                *(sram_ptr) = 1; // the "data-ready" flag

                // wait for FPGA to zero the "data_ready" flag
                while (*(sram_ptr)>0) ;

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
		VGA_PIXEL(x, y, c);
		//pixel_ptr = (char *)vga_pixel_ptr + (y<<10)+ x;
		// set pixel color
		//*(char *)pixel_ptr = c;

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

/// /// /////////////////////////////////////
/// end /////////////////////////////////////
              
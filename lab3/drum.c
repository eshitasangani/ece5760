///////////////////////////////////////
/// Audio
/// compile with
/// gcc drum.c -o testA -lm -O3 -pthread
/// works up to about drum size 30 or so for NO multiplies case
/// 
///////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
// interprocess comm
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <time.h>
// network stuff
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "address_map_arm_brl4.h"
#include <pthread.h>

#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

/* function prototypes */
void VGA_text (int, int, char *);
void VGA_box (int, int, int, int, short);
void VGA_line(int, int, int, int, short) ;

typedef signed int fix17 ;
//multiply two fixed 4:28
#define multfix17(a,b) ((fix17)(((( signed long long)(a))*(( signed long long)(b)))>>28)) 
//#define multfix28(a,b) ((fix28)((( ((short)((a)>>17)) * ((short)((b)>>17)) )))) 
#define float2fix17(a) ((fix17)((a)*131072.0f)) // 2^17
#define fix2float17(a) ((float)(a)/131072.0f) 
#define int2fix28(a) ((a)<<17)
#define fix2int28(a) ((a)>>17)

// the light weight buss base
void *h2p_lw_virtual_base;

// // pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// // character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// /dev/mem file descriptor
int fd;

fix17 pio_init;
int temp_rows;
float temp_init;
float temp_rho ;
float temp_step;
float temp_damping;

// measure time
struct timeval t1, t2;
double elapsedTime;
double elapsed_time; // for timer

//// BASE ADDRESSES FOR PIO ADDRESSES ////
#define PIO_RESET_BASE   		0x00000030
#define PIO_NUM_ROWS_BASE   	0x00000040
#define PIO_RHO_BASE   			0x00000050
#define PIO_DAMPING_BASE		0x00000060
#define PIO_DONE_DONE_BASE  	0x00000070
#define PIO_STEP_Y_BASE  		0x00000080

//// PIO ADDRESSES INITIALIZATION //// 
volatile unsigned int *pio_reset_addr 			= NULL;
volatile unsigned int *pio_num_rows_addr 		= NULL;
volatile unsigned int *pio_rho_addr 			= NULL;
volatile unsigned int *pio_damping_addr		 	= NULL;
volatile unsigned int *pio_done_done_addr 		= NULL;
volatile unsigned int *pio_step_y_addr 			= NULL;

int idx = 0;
int set = 0;

///////////////////////////////////////////////////////////////
// scan thread  // 
///////////////////////////////////////////////////////////////
void * scan_thread () { 
	while (1) {
		printf("0: number of rows; 1: change initial value; 2: change rho 3: damping \n");
		scanf("%i", &set);

		switch ( set ) {

			case 0:

				printf("Enter number of rows: ");
				scanf("%d", &temp_rows);
				*pio_num_rows_addr = temp_rows;
				*pio_step_y_addr =float2fix17(temp_init/temp_rows/2.0);
				*pio_reset_addr = 1;
				*pio_reset_addr = 0;

			break;

			case 1:

				printf("Enter initial value: ");
				scanf("%f", &temp_init);
				*pio_step_y_addr =float2fix17(temp_init/temp_rows/2.0);

				*pio_reset_addr = 1;
				*pio_reset_addr = 0;

			break;

			case 2:

				printf("Enter rho value: ");
				scanf("%f", &temp_rho);
				*pio_rho_addr = float2fix17(temp_rho);

				*pio_reset_addr = 1;
				*pio_reset_addr = 0;
			break;

			case 3:

				printf("Enter damping value: ");
				scanf("%f", &temp_damping);
				*pio_damping_addr = float2fix17(temp_damping);

				*pio_reset_addr = 1;
				*pio_reset_addr = 0;

			break;

		}

	}

}

///////////////////////////////////////////////////////////////
					//// prints time ////
///////////////////////////////////////////////////////////////

void print_stats(){

	while((*pio_done_done_addr)){}

	gettimeofday(&t1, NULL);

	while(!*pio_done_done_addr){}

	gettimeofday(&t2, NULL);

	elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000000.0;      // sec to us
	elapsed_time += (t2.tv_usec - t1.tv_usec) ;   // us 
	printf("\ntime: %.2f us ", elapsed_time);

}


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
	vga_pixel_virtual_base = mmap( NULL, FPGA_ONCHIP_SPAN, ( 	PROT_READ | PROT_WRITE ), MAP_SHARED, fd, 			FPGA_ONCHIP_BASE);	
	if( vga_pixel_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA pixel buffer
	//vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);


	pio_reset_addr 				= (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_BASE );
	pio_num_rows_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_NUM_ROWS_BASE );
	pio_rho_addr 				= (unsigned int *)(h2p_lw_virtual_base +  PIO_RHO_BASE );
	pio_damping_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_DAMPING_BASE );
	pio_done_done_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_DONE_DONE_BASE );
	pio_step_y_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_STEP_Y_BASE );

    // inital values to the fpga

	*pio_num_rows_addr 		= 160;
	*pio_rho_addr			= float2fix17(0.125);
	*pio_damping_addr		= 11;
	*pio_step_y_addr		= float2fix17(0.15/80); // 80 is num rows/2 (init to 160 rows)
	 
	idx = 0;
	set = 0;
	pio_init 	= 0;
	temp_rows 	= 150;
	temp_init 	= 0.0625;
	temp_rho 	= 0.25;
	temp_step 	= 0.002;
	

	// I DONT REALLY GET IT BUT SOMETIMES IT BREAKS YOUR EARDRUMS GOODLUCK ESHITA

	while (1) {
		printf("0: number of rows; 1: change initial value; 2: change rho 3: damping \n");
		scanf("%i", &set);

		switch ( set ) {

			case 0:

				printf("Enter number of rows: ");
				scanf("%d", &temp_rows);
				*pio_num_rows_addr = temp_rows;
				*pio_step_y_addr =float2fix17(temp_init/(temp_rows/2.0));
				print_stats();

				// *pio_reset_addr = 1;
				// *pio_reset_addr = 0;
				// *pio_reset_addr = 1;
			break;

			case 1:

				printf("Enter initial value: ");
				scanf("%f", &temp_init);
				*pio_step_y_addr =float2fix17(temp_init/(temp_rows/2.0));
				print_stats();

				// *pio_reset_addr = 1;
				// *pio_reset_addr = 0;
				// *pio_reset_addr = 1;

			break;

			case 2:

				printf("Enter rho value: ");
				scanf("%f", &temp_rho);
				*pio_rho_addr = float2fix17(temp_rho);
				print_stats();

				// *pio_reset_addr = 1;
				// *pio_reset_addr = 0;
				// *pio_reset_addr = 1;


			break;

			// case 3:

			// 	printf("Enter damping value: ");
			// 	scanf("%f", &temp_damping);
			// 	*pio_damping_addr = float2fix17(temp_damping);

			// 	*pio_reset_addr = 1;
			// 	*pio_reset_addr = 0;
			// 	*pio_reset_addr = 1;
			// 	print_stats();

			// break;
			

		}


	}

	
	return 0;

}
		
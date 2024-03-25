///////////////////////////////////////
/// Audio
/// compile with
/// gcc drum.c -o testA -lm -O3
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

#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

/* function prototypes */
void VGA_text (int, int, char *);
void VGA_box (int, int, int, int, short);
void VGA_line(int, int, int, int, short) ;

typedef signed int fix17 ;
//multiply two fixed 4:28
#define multfix17(a,b) ((fix17)(((( signed long long)(a))*(( signed long long)(b)))>>28)) 
//#define multfix28(a,b) ((fix28)((( ((short)((a)>>17)) * ((short)((b)>>17)) )))) 
#define float2fix17(a) ((fix17)((a)*131072.0f)) // 2^28
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

//// BASE ADDRESSES FOR PIO ADDRESSES ////
#define PIO_INIT_BASE       	0x00000000 
#define PIO_INIT_DONE_BASE  	0x00000010 
#define PIO_INIT_VAL_BASE   	0x00000020 
#define PIO_RESET_BASE   		0x00000030
#define PIO_NUM_ROWS_BASE   	0x00000040
#define PIO_RHO_BASE   			0x00000050
#define PIO_INITIAL_VALUE_BASE  0x00000060




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

    //// PIO ADDRESSES INITIALIZATION //// 
    volatile unsigned int *pio_init_addr 			= NULL;
    volatile unsigned int *pio_init_done_addr 		= NULL;
	volatile unsigned int *pio_init_val_addr 		= NULL;
	volatile unsigned int *pio_reset_addr 			= NULL;
	volatile unsigned int *pio_num_rows_addr 		= NULL;
	volatile unsigned int *pio_rho_addr 			= NULL;
	volatile unsigned int *pio_initial_value_addr 	= NULL;

    pio_init_addr   			= (unsigned int *)(h2p_lw_virtual_base +  PIO_INIT_BASE );
    pio_init_done_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_INIT_DONE_BASE );
	pio_init_val_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_INIT_VAL_BASE );
	pio_reset_addr 				= (unsigned int *)(h2p_lw_virtual_base +  PIO_RESET_BASE );
	pio_num_rows_addr 			= (unsigned int *)(h2p_lw_virtual_base +  PIO_NUM_ROWS_BASE );
	pio_rho_addr 				= (unsigned int *)(h2p_lw_virtual_base +  PIO_RHO_BASE );
	pio_initial_value_addr 		= (unsigned int *)(h2p_lw_virtual_base +  PIO_INITIAL_VALUE_BASE );


    // inital values to the fpga
    //*pio_init_addr = int2fix28(0);
	//*pio_init_val_addr = 0;
	*pio_num_rows_addr 		= 450
	;
	*pio_rho_addr			= float2fix17(0.125);
	*pio_initial_value_addr	= float2fix17(0.0625);
	int idx = 0;
	int set = 0;

	fix17 pio_init 	= 0;
	int temp_rows 	= 150;
	float temp_init = 0.0625;
	float temp_rho 	= 0.25;

	while (1) {
		printf("Enter number of rows: ");
		scanf("%d", &temp_rows);
		*pio_num_rows_addr = temp_rows;

		*pio_reset_addr = 1;
		*pio_reset_addr = 0;
		*pio_reset_addr = 1;

	}

	// I DONT REALLY GET IT BUT SOMETIMES IT BREAKS YOUR EARDRUMS GOODLUCK ESHITA

	// while (1) {
	// 	printf("0: number of rows; 1: change initial value; 2: change rho \n");
    // 	scanf("%i", &set);

	// 	switch ( set ) {

	// 		case 0:

	// 			printf("Enter number of rows: ");
	// 			scanf("%d", &temp_rows);
	// 			*pio_num_rows_addr = temp_rows;

	// 			*pio_reset_addr = 1;
	// 			*pio_reset_addr = 0;
	// 			*pio_reset_addr = 1;

	// 		break;

	// 		case 1:

	// 			printf("Enter initial value: ");
	// 			scanf("%f", &temp_init);

	// 			*pio_initial_value_addr = float2fix17(temp_init);

	// 			*pio_reset_addr = 1;
	// 			*pio_reset_addr = 0;
	// 			*pio_reset_addr = 1;

	// 		break;

	// 		case 2:

	// 			printf("Enter rho value: ");
	// 			scanf("%f", &temp_rho);

	// 			*pio_rho_addr = float2fix17(temp_rho);

	// 			*pio_reset_addr = 1;
	// 			*pio_reset_addr = 0;
	// 			*pio_reset_addr = 1;

	// 		break;

	// 	}


	// }

    // while (1) { 

    //     // if (*pio_init_done_addr) { 
	// 	// if (idx < 30) {
	// 	// 	printf("%d", idx);
	// 	// }
		
	// 	if (*pio_reset_addr){
	// 		*pio_init_addr = int2fix28(0);
	// 		idx = 0;
	// 	}

	// 	if (idx <= 14) { 
	// 		pio_init += float2fix17(0.0078125);
	// 		*pio_init_addr = pio_init;
	// 		*pio_init_val_addr = 1;
	// 		idx += 1; 
	// 		*pio_init_val_addr = 0;
	// 	}

	// 	if (idx == 15) { 
	// 		*pio_init_addr = pio_init;
	// 		*pio_init_val_addr = 1;
	// 		idx += 1; 
	// 		*pio_init_val_addr = 0;
	// 	}

	// 	if (idx > 15 && idx < 30) {
	// 		pio_init -= float2fix17(0.0078125);
	// 		*pio_init_addr = pio_init;
	// 		*pio_init_val_addr = 1;
	// 		idx += 1;
	// 		*pio_init_val_addr = 0;
	// 	}

	// 	// if (16 <= idx < 30) {
	// 	// 	pio_init -= float2fix17(0.0078125);
	// 	// 	*pio_init_addr = pio_init;
	// 	// 	// *pio_init_val_addr = 1;
	// 	// 	idx += 1;
	// 	// 	// *pio_init_val_addr = 0;
	// 	// }
	// 	// printf("%d", idx);

    //     // }
		

    // }

} 
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

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// /dev/mem file descriptor
int fd;

//// BASE ADDRESSES FOR PIO ADDRESSES ////
#define PIO_INIT_BASE        0x00000000 
#define PIO_INIT_DONE_BASE   0x00000010 


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
	vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

    //// PIO ADDRESSES INITIALIZATION //// 
    volatile unsigned int *pio_init_addr = NULL;
    volatile unsigned int *pio_init_done_addr = NULL;

    pio_init_addr   = (unsigned int *)(h2p_lw_virtual_base +  PIO_INIT_BASE );
    pio_init_done_addr = (unsigned int *)(h2p_lw_virtual_base +  PIO_INIT_DONE_BASE );

    // inital values to the fpga
    *pio_init_addr = int2fix28(0);

    while (1) { 

        int idx = 0;

        if (*pio_init_done_addr) { 

            if (idx <= 14) { 
                *pio_init_addr = *pio_init_addr + float2fix17(0.0078125);
                idx++; 
            }

            else if (idx == 15) { 
                *pio_init_addr = *pio_init_addr;
                idx++; 
            }

            else if (idx > 15 && idx < 30) {
                *pio_init_addr = *pio_init_addr - float2fix17(0.0078125);
                idx++;
            }

        }

    }

} 
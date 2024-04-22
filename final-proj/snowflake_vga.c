///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc snowflake_vga.c -o snow
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

// fixed pt
typedef signed int fix28 ;
//multiply two fixed 4:28
#define multfix28(a,b) ((fix28)(((( signed long long)(a))*(( signed long long)(b)))>>28)) 
//#define multfix28(a,b) ((fix28)((( ((short)((a)>>17)) * ((short)((b)>>17)) )))) 
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
int i,j,k;

///////////////////////////////////////////////// 
#define WIDTH 51
#define HEIGHT 51
#define ALPHA 1
#define BETA 0.8
#define GAMMA 0.01
#define NUM_NEIGHBORS 6

typedef struct {
    float u;   // water used in diffusion
    float v;   // water not used in diffusion
    float s;   // total water
    bool is_receptive;
} Cell;

Cell cells[WIDTH][HEIGHT];
float s_vals[WIDTH][HEIGHT]; // Array to store s values for visualization or debugging
Cell* neighbors[NUM_NEIGHBORS];
int num_neighbors;

// 8-bit color
#define rgb(r,g,b) ((((r)&7)<<5) | (((g)&7)<<2) | (((b)&3)))

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	char  *pixel_ptr ;\
	pixel_ptr = (char *)vga_pixel_ptr + ((y)<<10) + (x) ;\
	*(char *)pixel_ptr = (color);\
} while(0)

// Get neighbors for a specific coordinate
int get_neighbors(Cell* neighbors[], int x, int y) {
	// SET EDGE CELLS TO BE STATIC ??
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

// // Get neighbors for a specific coordinate
// int get_neighbors(Cell* neighbors[], int x, int y) {
//     int count = 0;
 
//     // Check left neighbor
//     if (x > 0) {
//         neighbors[count++] = &cells[x-1][y]; // Direct left
//         if (y > 0) neighbors[count++] = &cells[x-1][y-1]; // Top-left
//         if (y < HEIGHT - 1) neighbors[count++] = &cells[x-1][y+1]; // Bottom-left
//     }

//     // Check right neighbor
//     if (x < WIDTH - 1) {
//         neighbors[count++] = &cells[x+1][y]; // Direct right
//         if (y > 0) neighbors[count++] = &cells[x+1][y-1]; // Top-right
//         if (y < HEIGHT - 1) neighbors[count++] = &cells[x+1][y+1]; // Bottom-right
//     }

//     // Check top and bottom neighbors
//     if (y > 0) neighbors[count++] = &cells[x][y-1]; // Direct top
//     if (y < HEIGHT - 1) neighbors[count++] = &cells[x][y+1]; // Direct bottom

//     return count;
// }

// void initialize_grid() {
//     for ( i = 0; i < WIDTH; i++) {
//         for ( j = 0; j < HEIGHT; j++) {
//             cells[i][j].s = BETA;
//             cells[i][j].is_receptive = false;
//             cells[i][j].u = 0;
//             cells[i][j].v = 0;
//         }
//     }
//     // Set the center cell
//     cells[WIDTH/2][HEIGHT/2].s = 1.0;
//     cells[WIDTH/2][HEIGHT/2].is_receptive = true;
// }



void update_s_vals() {
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < HEIGHT; j++) {
            s_vals[i][j] = cells[i][j].s;
        }
    }
}

void print_s_vals() {
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < HEIGHT; j++) {
            printf("%.2f ", s_vals[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_u_vals() {
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < HEIGHT; j++) {
			if (cells[i][j].u != 0.800000) { 
				printf("%f", cells[i][j].u);
				printf("\n");	
			}
			else {
				printf("%f", cells[25][25].u);
			}
            
        }
        printf("\n");
    }
    printf("\n");
}


bool any_frozen = false;
float u_avg = 0.0;
float sum_u = 0.0;
float interm = 0.0;

// int count = 0;

void one_iter() {

	// initialize cell struct grid first 
	for ( i = 0; i < 51; i++) {
        for ( j = 0; j < 51; j++) {
            cells[i][j].s = BETA;
            cells[i][j].is_receptive = false;
            cells[i][j].u = 0;
            cells[i][j].v = 0;
        }
    }
    // Set the center cell
    cells[25][25].s = 1.0;
    cells[25][25].is_receptive = true;

    // Determine receptive sites
    for ( i = 0; i < 51; i++) {
        for ( j = 0; j < 51; j++) {
			// count++;
			// printf("%d", count);
			// printf("\n");
			// printf("%d ", i);
			// printf("%d ", j);
			// printf("\n");
            if (cells[i][j].is_receptive) {
                cells[i][j].u = 0;
                cells[i][j].v = cells[i][j].s;
                cells[i][j].s = cells[i][j].v + GAMMA;
            } 
			else {
                cells[i][j].u = cells[i][j].s;
                cells[i][j].v = 0;
            }
        }
		// printf("%.2f ", i);
		// printf("%.2f ", j);
    }



    // Diffusion process
	for (i = 0; i < 51; i++) {
		for (j = 0; j < 51; j++) {
			printf("%d ", i);
			printf("%d ", j);
			// printf("\n");
			
			num_neighbors = get_neighbors(neighbors, i, j);
			printf("%d ", num_neighbors);
			printf("\n");
			if (num_neighbors > 0) { // make sure there are neighbors so we can avoid division bt 0
				sum_u = 0;

				for (k = 0; k < num_neighbors; k++) {
					sum_u += neighbors[k]->u; // Sum u values of all neighbors
				}

				u_avg = sum_u / num_neighbors; // Calculate average u
				// printf("%f", u_avg);
				// interm = (u_avg - cells[i][j].u );
				cells[i][j].u += 0.5 ;
				
				// printf("%f", cells[i][j].u);
				
				// cells[i][j].u = cells[i][j].u + (ALPHA / 2 * (u_avg - cells[i][j].u));
				// cells[i][j].u += ALPHA / 2 * (u_avg - cells[i][j].u); // Update u based on the diffusion equation
				// cells[i][j].s = cells[i][j].u + cells[i][j].v; // Update total s

			// 	// Update receptiveness based on the new sp
			// 	if (cells[i][j].s >= 1) {
			// 		cells[i][j].is_receptive = true;
			// 	} 
			// 	// ^^ that if statement is causing the seg fault? im not sure why tho 

			// 	else {
			// 		any_frozen = false;
			// 		for (k = 0; k < num_neighbors; k++) {
			// 			if (neighbors[k]->s >= 1) {
			// 				any_frozen = true;
			// 				break;
			// 			}
			// 		}
			// 		cells[i][j].is_receptive = any_frozen;
			// 	}

			} 
			// end of if statment checking to make sure if we have neighbors or not 

			// else { // if cell has no neighbors, just make it so that it is not receptive

			// 	cells[i][j].is_receptive = false;
			// }
		}
	}
 }



void draw_VGA_test(){
	// index by the cell numbers
	for (i = 1; i < 640; i++) {  // column number (x)
		for (j = 1; j < 480; j++ ) { // row number (y)
			// void VGA_box(int x1, int y1, int x2, int y2, short pixel_color)
			if (j % 2 == 0) {
				// VGA_PIXEL(2*i, 2*(j-1), 2*(i+2), 2*(j-3), 0x1d);
				VGA_box(i-1, j-1, i+1, j+1, 0x1d);
			}  
			else{
				VGA_box(i, j, i-1, j-1, 0x1b);
			}
		}
	}
}

// need to map the # of neighbors to the columns on the vga 

// void run_snow() {
// 	// this runs snowflake gen for 1 iteration + updates the cells 
// 	// one_iter();
// 	// update s array
// 	// update_s_vals();

// 	for (i = 0; i < WIDTH; i++) { 
// 		for (j = 0; j < HEIGHT; j++){
// 			if (s_vals[i][j] >= 1 ) { 
// 				// means it is frozen 
// 				color = 0x1d;
// 				// VGA_box(i-1, j-1, i+1, j+1, 0x1d);
// 			}
// 			else { 
// 				color = 0x1c;
// 				// VGA_box(i, j, i-1, j-1, 0x1b);
// 			}

// 			// draw even // odd columns 
// 		}
// 	}

	// now actually draw on the vga 
	// for (i = 1; i < 640; i++) {  // column number (x)
	// 	for (j = 1; j < 480; j++ ) { // row number (y)
	// 		// void VGA_box(int x1, int y1, int x2, int y2, short pixel_color)
	// 		if (j % 2 == 0) {
	// 			// VGA_PIXEL(2*i, 2*(j-1), 2*(i+2), 2*(j-3), 0x1d);
	// 			VGA_box(i-1, j-1, i+1, j+1, color);
	// 		}  
	// 		else{
	// 			VGA_box(i, j, i-1, j-1, color);
	// 		}
	// 	}
	// }

// }

// Define the size of a square cell in pixels
// void draw_snowflakes() {
//     // one_iter(); // update the states

//     for ( i = 0; i < WIDTH; i++) {
//         for ( j = 0; j < HEIGHT; j++) {
//             // top-left corner of the square for this cell
//             int x = i ;
//             int y = j ;

//             // If odd row, add half the cell width to x
//             if (j % 2 != 0) {
//                 x += 4;
//             }

//             // determine the color based on whether the cell is frozen
// 			// white for frozen, black otherwise
//             short color = cells[i][j].is_receptive ? rgb(3, 3, 3) : rgb(0, 0, 0); 

// 			// draw onto the vga 
//             VGA_box(x, y, x + 9, y + 9, color);
//         }
//     }
// }

// width is 12 height is 9 
// mapping 51x51 grid to a 640 by 480 grid 

// void draw_snowflakes() {
//     one_iter(); 
//     update_s_vals();  

//     for (int i = 0; i < WIDTH; i++) {
//         for (int j = 0; j < HEIGHT; j++) {
//             int x1 = i * 12;
//             int y1 = j * 9;
//             int x2 = x1 + 12 - 1;
//             int y2 = y1 + 9 - 1;
//             short color = (s_vals[i][j] >= 1) ? rgb(7, 7, 7) : rgb(0, 0, 0);  // White or black

//             VGA_box(x1, y1, x2, y2, color);
//         }
//     }
// }

/// even odd columbs based on the neighbors
// loops through the s values
// void run_snow() {
//     one_iter(); 
//     update_s_vals();  // Update s values for drawing

//     // Loop through s_vals to draw each cell on the VGA screen
//     for ( i = 0; i < WIDTH; i++) {
//         for ( j = 0; j < HEIGHT; j++) {
//             int x1 = i * 2;  // X start pixel
//             int y1 = j * 2;  // Y start pixel

//             // Adjust for odd columns
//             if (j % 2 == 1) {
//                 y1++;
//             }

//             // Set the color based on the cell's state
// 			// frozen cells are white and others are black 
//             short color = (s_vals[i][j] >= 1) ? rgb(3, 3, 3) : rgb(0, 0, 0);

//             // Draw the cell as a 2x2 pixel square
//             VGA_box(x1, y1, x1 + 1, y1 + 1, color);
//         }
//     }
// }


int main(void)
{
	//int x1, y1, x2, y2;

	// Declare volatile pointers to I/O registers (volatile 	// means that IO load and store instructions will be used 	// to access these pointer locations, 
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

	// ===========================================

	/* create a message to be displayed on the VGA 
          and LCD displays */
	char text_top_row[40] = "DE1-SoC ARM/FPGA\0";
	char text_bottom_row[40] = "Cornell ece5760\0";

	//VGA_text (34, 1, text_top_row);
	//VGA_text (34, 2, text_bottom_row);
	// clear the screen
	// VGA_box (0, 0, 639, 479, 0x1c);
	VGA_box (0, 0, 639, 479, 0x3a);


    // draw_snowflakes(); 

	// run_snow();
	 one_iter();
	// update_s_vals();
	// print_u_vals();
	printf("%f", cells[25][25].u);
	printf("%f", cells[25][25].v);
	printf("%f", cells[25][25].s);
	// clear the text
	// VGA_text_clear();

    // VGA_text (10, 1, text_top_row);
    // VGA_text (10, 2, text_bottom_row);


    
	//} // end while(1)
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
			pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
			// set pixel color
			*(char *)pixel_ptr = pixel_color;		
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
				pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
				// set pixel color
				*(char *)pixel_ptr = pixel_color;
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
		pixel_ptr = (char *)vga_pixel_ptr + (y<<10)+ x; 
		// set pixel color
		*(char *)pixel_ptr = c;	
		 
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
#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "io.h"
#include "images.h"

#define DEBUG 1
#define STEP_RESULT 1

#define SECTION_LOAD_TO_SM 1
#define SECTION_GRAYSCALE 2
#define SECTION_RESIZE 3
#define SECTION_ASCII_IMAGE 4
#define SECTION_SAVE_TO_SRAM 5
#define TOTAL_EXECUTION_TIME 6

#define SIZE_X SHARED_ONCHIP_BASE+8184
#define SIZE_Y SHARED_ONCHIP_BASE+8185
#define COLOR_SCALE SHARED_ONCHIP_BASE+8186

#define ASCII_IMAGE_BASE SHARED_ONCHIP_BASE+5120
#define ASCII_IMAGE_SYMBOL_BASE SHARED_ONCHIP_BASE+6144
/*
 * This is a function to perform Delay!
 */

extern void delay (int millisec);

/*
 * This is the function for abs
 */
int abs(int value)
{
    if (value>=0) return value;
    else return -value;
}


/*
 * Example function for copying a p3 image from sram to the shared on-chip mempry
 */
void sram2sm_p3(unsigned char* base)
{
    #if DEBUG==1
    int x, y;
    unsigned char* shared = (unsigned char*) SHARED_ONCHIP_BASE;
    IOWR_8DIRECT(SIZE_X,0,*base);
    int size_x = *base++;
    IOWR_8DIRECT(SIZE_Y,0,*base);
    int size_y = *base++;
    IOWR_8DIRECT(COLOR_SCALE,0,*base);
    base++;
    printf("The image is: %d x %d!! \n", size_x, size_y);
    for(y = 0; y < size_y; y++)
	for(x = 0; x < size_x; x++)
	{
	    *shared++ = *base++; 	// R
	    *shared++ = *base++;	// G
	    *shared++ = *base++;	// B
	}
    #else
    base=base+3;//Begin to access the RGB value of the image.
    int* source32bit = (int*) base;
    int* destination32bit = (int*) SHARED_ONCHIP_BASE;
    int i;
    for (i=0;i<768;i++)
    {
	IOWR_32DIRECT(destination32bit,(i<<2),*(source32bit+i));

    }
    #endif

}


/*
 * This is the function that loads ASCII symbol to onm-chip memory
 */
void loadASCIISymbol()
{
    unsigned char i;
    unsigned char symbols[] = {' ','\'','=','!','L','T','{','x','e','n','V','G','R','m','Q','@'};
    for (i=0;i<16;i++)
    {
	IOWR_8DIRECT(ASCII_IMAGE_SYMBOL_BASE,i,symbols[i]);
    }
}

/*
 *  This is the function that save the ASCII image to SRAM.
 */


void sm2sram(unsigned char* destination)
{
    unsigned char i;
    #if DEBUG == 1
    unsigned char j,size_x,size_y;
    unsigned char* asciiImagePtr = (unsigned char*) ASCII_IMAGE_BASE;
    size_x=IORD_8DIRECT(SIZE_X,0);
    size_y=IORD_8DIRECT(SIZE_Y,0);
    *destination=size_x;
    destination++;
    *destination=size_y;
    destination++;
    *destination=16;
    destination++;
    for (i=0;i<size_y;i++)
    {
	for (j=0;j<size_x;j++)
	{
	    *destination=*asciiImagePtr;
	    destination++;
	    asciiImagePtr++;
	}
    }
    #else
    int* sramPtr32bit = (int*) destination;
    *sramPtr32bit = 0x00100D0D; 
    // The nios ii processor is little endian, so the size_x and size_y need to be stored in lower bits.
    int* smPtr32bit = (int*) ASCII_IMAGE_BASE;
    destination=destination+3;    
    sramPtr32bit = (int*) destination;
    //There are 14*14 pixels in sum and every time we load 4 pixels. So the counter is ended with 49. 
    for (i=0;i<49;i++)
    {
	*sramPtr32bit=IORD_32DIRECT(smPtr32bit,(i<<2));
	sramPtr32bit++;
    }
    #endif
}

int main()
{
    char current_image=0;
    unsigned char counter=1;
    #if DEBUG == 1
    /* Sequence of images for testing if the system functions properly */
    char number_of_images=10;
    unsigned char* img_array[10] = {img1_24_24, img1_32_22, img1_32_32, img1_40_28, img1_40_40, img2_24_24, img2_32_22, img2_32_32, img2_40_28, img2_40_40 };
    /* This is for debugging propose to limit the iteraion.*/
    
    #else
    /* Sequence of images for measuring performance */
    char number_of_images=3;
    unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
    #endif
    loadASCIISymbol();
    unsigned char asciiImageStore[400];
    while (counter)
    {
	#if DEBUG == 1
	/* Just to see that the task compiles correctly */
	counter=counter-1;
	#endif
	
	
	
	/* Extract the x and y dimensions of the picture */
	unsigned char x,y;
	unsigned char* shared;
	unsigned char size_x, size_y;
	   
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
	
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME);
		
	
	/*
	 * This is the part that loads RGB image to shared memory.
	 */    
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_LOAD_TO_SM);
	sram2sm_p3(img_array[current_image]);
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_LOAD_TO_SM);
	
	
	
	
	/*
	 * This is the part that generates gray scale image
	 */
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_GRAYSCALE);
	unsigned char* greyscalePos;
	unsigned int temp;
	shared = (unsigned char*) SHARED_ONCHIP_BASE;
	greyscalePos = (unsigned char*) SHARED_ONCHIP_BASE;
	#if DEBUG == 1
	size_x = IORD_8DIRECT(SIZE_X,0);
	size_y = IORD_8DIRECT(SIZE_Y,0);
	for (y=0 ;y<size_y; y++)
	{
	    for (x=0 ;x<size_x; x++)
	    {
		temp=((*shared)<<2)+(*shared);
		shared = shared + 1;
		temp=((*shared)<<3)+(*shared)+temp;
		shared = shared + 1;
		temp=((*shared)<<1)+temp;
		temp=temp>>4;
		*greyscalePos = (unsigned char) temp;		
		shared = shared + 1;
		greyscalePos = greyscalePos + 1;
		
	    }
	}
	#else
	size_x=32;
	size_y=32;
	for (y=0 ;y<32; y++)
	{
	    for (x=0 ;x<32; x=x+1)
	    {
		temp=((*shared)<<2)+(*shared);
		shared = shared + 1;
		temp=((*shared)<<3)+(*shared)+temp;
		shared = shared + 1;
		temp=((*shared)<<1)+temp;
		temp=temp>>4;
		*greyscalePos = (unsigned char) temp;	    
		shared = shared + 1;
		greyscalePos = greyscalePos + 1;
		
	    }
	}
	#endif // DEBUG
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_GRAYSCALE);
	#if STEP_RESULT==1
	printf("Grayscale:\n");
	for (y=0 ;y<size_y; y++)
	{
	    for (x=0 ;x<size_x; x++)
	    {
		printf("%d\t",IORD_8DIRECT(SHARED_ONCHIP_BASE,size_x*y+x));
	    }
	    printf("\n");
	}
	#endif
	
	
	
	
	
	
	
	/*
	 *  This is the part that resizes the image
	 */
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,SECTION_RESIZE);
	unsigned char* resizedImagePos = (unsigned char*) SHARED_ONCHIP_BASE;;
	shared = SHARED_ONCHIP_BASE;
	#if DEBUG == 1
	for (y=0; y<size_y; y=y+2)
	{
	    for (x=0; x<size_x; x=x+2)
	    {
		*resizedImagePos=(*(shared+size_x*(y+1)+x+1)+*(shared+size_x*(y+1)+x)+*(shared+size_x*y+x+1)+(*(shared+size_x*y+x)))>>2;
		resizedImagePos = resizedImagePos + 1;
	    }
	}
	size_x=size_x>>1;
	size_y=size_y>>1;
	IOWR_8DIRECT(SIZE_X,0,size_x);
	IOWR_8DIRECT(SIZE_Y,0,size_y);
	#else
	for (y=0; y<32; y=y+2)
	{
	    for (x=0; x<32; x=x+2)
	    {
		*resizedImagePos=(*(shared+((y+1)<<5)+x+1)+*(shared+((y+1)<<5)+x)+*(shared+(y<<5)+x+1)+(*(shared+(y<<5)+x)))>>2;
		resizedImagePos = resizedImagePos + 1;
	    }
	}
	size_x=16;
	size_y=16;
	#endif
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_RESIZE);
	#if STEP_RESULT==1
	printf("Resized Image:\n");
	for (y=0 ;y<size_y; y++)
	{
	    for (x=0 ;x<size_x;x++)
	    {
		printf("%d\t",IORD_8DIRECT(SHARED_ONCHIP_BASE,size_x*y+x));
	    }
	    printf("\n");
	}
	#endif
	
	
	
	
	
	
	/*
	 * This part is to generate the ASCII image
	 */
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_ASCII_IMAGE);
	int sobelOfPixel,Gx,Gy; 
	unsigned char *pixel = (unsigned char*) SHARED_ONCHIP_BASE;
	unsigned char* asciiImage = (unsigned char*) ASCII_IMAGE_BASE;
	#if DEBUG == 1	
	for (y=0; y<size_y; y=y+2)
	{
	    for (x=0; x<size_x; x=x+2)
	    {
		Gx=((*(pixel+1))<<1)+(*(pixel+1-size_x))-((*(pixel-1))<<1)-(*(pixel-size_x-1))+(*(pixel+1+size_x))-(*(pixel+size_x-1));
		Gy=-((*(pixel-size_x))<<1)-(*(pixel-size_x+1))-(*(pixel-size_x-1))+((*(pixel+size_x))<<1)+(*(pixel+size_x+1))+(*(pixel+size_x-1));
		sobelOfPixel=(abs(Gx)+abs(Gy))/100;
		if (sobelOfPixel>15)
		{
		    *asciiImage='@';
		}
		else
		{
		    *asciiImage=IORD_8DIRECT(ASCII_IMAGE_SYMBOL_BASE,sobelOfPixel);
		}
		pixel++;
		asciiImage++;
	    }
	}
	#else
	pixel= (unsigned char*) (RESIZED_IMAGE_BASE+15);
	for (y=0;y<14;y++)
	{
	    pixel=pixel+2;
	    for (x=1;x<15;x++)
	    {		    
		Gx=((*(pixel+1))<<1)+(*(pixel-15))-((*(pixel-1))<<1)-(*(pixel-17))+(*(pixel+17))-(*(pixel+15));
		Gy=-((*(pixel-16))<<1)-(*(pixel-15))-(*(pixel-17))+((*(pixel+16))<<1)+(*(pixel+17))+(*(pixel+15));
		sobelOfPixel=(abs(Gx)+abs(Gy))/100;
		if (sobelOfPixel>15)
		{
		    *asciiImage='@';
		}
		else
		{
		    *asciiImage=IORD_8DIRECT(ASCII_IMAGE_SYMBOL_BASE,sobelOfPixel);
		}
		pixel++;
		asciiImage++;
	    }
	#endif
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_ASCII_IMAGE);
	    
	    
	
	
	/*
	 * This part is to save ASCII image to SRAM
	 */
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,SECTION_SAVE_TO_SRAM);
	sm2sram(asciiImageStore);
	PERF_END(PERFORMANCE_COUNTER_0_BASE,SECTION_SAVE_TO_SRAM);
	
	
	PERF_END(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME);
	
	
	#if STEP_RESULT==1
	printf("The symbol table is:\n");
	unsigned char i;
	for (i=0;i<16;i++)
	{
	    printf("%c\t",IORD_8DIRECT(ASCII_IMAGE_SYMBOL_BASE,i));
	}
	printf("\n");
	#endif
	
	
	
	printf("ASCII Image:\n");
	for (y=0 ;y<size_y; y++)
	{
	    for (x=0 ;x<size_x; x=x+3)
	    {
		printf("%c ",IORD_8DIRECT(ASCII_IMAGE_BASE,size_x*y+x));
	    }
	    printf("\n");
	}
	
	/* Print report */
	perf_print_formatted_report
	(PERFORMANCE_COUNTER_0_BASE,
	ALT_CPU_FREQ,        // defined in "system.h"
	6,                   // How many sections to print
	"Copy to SRAM",        // Display-name of section(s).
	"Greyscale",
	"Resizing",
	"Computing Sobels",
	"Save to SRAM",
	"Total"
	);
	
	
	
	
	
	current_image=(current_image+1) % number_of_images;
	delay (300);    
    }
	
    return 0;
}
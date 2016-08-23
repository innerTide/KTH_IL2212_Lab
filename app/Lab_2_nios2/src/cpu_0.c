#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "io.h"
#include "images.h"

#define DEBUG 0

#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3
#define SECTION_4 4

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
    int x, y;
    unsigned char* shared;
    
    shared = (unsigned char*) SHARED_ONCHIP_BASE;
    
    int size_x = *base++;
    int size_y = *base++;
    int max_col= *base++;
    *shared++  = size_x;
    *shared++  = size_y;
    *shared++  = max_col;
    printf("The image is: %d x %d!! \n", size_x, size_y);
    for(y = 0; y < size_y; y++)
	for(x = 0; x < size_x; x++)
	{
	    *shared++ = *base++; 	// R
	    *shared++ = *base++;	// G
	    *shared++ = *base++;	// B
	}
}

/*
 * This is the function of generating grey scale image
 */

void greyscaleGen()
{
    unsigned char* shared;
    unsigned char* greyscalePos;
    unsigned char x;
    unsigned char y;
    unsigned int temp;
    shared = (unsigned char*) SHARED_ONCHIP_BASE;
    greyscalePos = (unsigned char*) SHARED_ONCHIP_BASE;
    unsigned char size_x = (*shared)*3;
    unsigned char size_y = *(shared+1);
    shared = shared + 3;
    greyscalePos = greyscalePos + 3;
    #if DEBUG == 1
    printf("\nGreyscaled Image is \n");
    #endif // DEBUG
    for (y=0 ;y<size_y; y++)
    {
	for (x=0 ;x<size_x; x=x+3)
	{
	    temp=((*shared)<<2)+(*shared);
	    shared = shared + 1;
	    temp=((*shared)<<3)+(*shared)+temp;
	    shared = shared + 1;
	    temp=((*shared)<<1)+temp;
	    temp=temp>>4;
	    *greyscalePos = (unsigned char) temp;
	    
	    shared = shared + 1;
	    #if DEBUG == 1
	    printf("%d\t",*greyscalePos);
	    #endif // DEBUG
	    greyscalePos = greyscalePos + 1;
	    
	}
	#if DEBUG == 1
	printf("\n");
	#endif // DEBUG
    }
}

/*
 * This is the function to resize the image in the shared memory
 */

void resizeImage()
{
    unsigned char* shared;
    unsigned char* resizedImagePos;
    unsigned char x,y;
    shared = (unsigned char*) SHARED_ONCHIP_BASE;
    resizedImagePos = (unsigned char*) SHARED_ONCHIP_BASE;
    unsigned char size_x = *shared;
    unsigned char size_y = *(shared+1);
    *shared=(*shared)>>1;
    shared = shared + 1;
    *shared=(*shared)>>1;
    shared = shared + 2;
    resizedImagePos = resizedImagePos + 3;
    #if DEBUG == 1
    printf("\nResized Image is: \n");
    #endif // DEBUG
    for (y=0; y<size_y; y=y+2)
    {
	for (x=0; x<size_x; x=x+2)
	{
	    *resizedImagePos=(*(shared+size_x*(y+1)+x+1)+*(shared+size_x*(y+1)+x)+*(shared+size_x*y+x+1)+(*(shared+size_x*y+x)))>>2;
	    #if DEBUG == 1
	    printf("%d\t",*resizedImagePos);
	    #endif // DEBUG
	    resizedImagePos = resizedImagePos + 1;
	}
	#if DEBUG == 1
	printf("\n");
	#endif // DEBUG
    }
    
}

/*
 * This is the function to perform edge detection using Sobel operator, it needs to be mentioned that size_x and size_y need to make a decreasement
 * Arguement List:
 **sourcePixelPtr: A pointer points to a pixel of source image;
 **xPos: the X position of the pixel to be computed;
 **yPos: the Y position of the pixel to be computed;
 **size_x: the actual X size of source image minus 1;
 **size_y: the actual Y size of source image minus 1;
 */

int computeSobel(unsigned char* sourcePixelPtr, unsigned char xPos, unsigned char yPos , unsigned char size_x, unsigned char size_y)
{
    int Gx,Gy;
    if (yPos == 0)
    {
	if (xPos == 0)
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1+size_x));
	    Gy=2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1));
	    return Gx+Gy;
	}
	else if (xPos == (size_x-1))
	{
	    Gx=2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr+size_x-1));
	    Gy=2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x-1));
	    return Gx+Gy;
	}
	else
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1+size_x))-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr+size_x-1)));
	    Gy=2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1))+(*(sourcePixelPtr+size_x-1));
	    return abs(Gx)+Gy;
	}
    }
    else if (yPos == (size_y-1))
    {
	if (xPos == 0)
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x));
	    Gy=2*(*(sourcePixelPtr-size_x))+(*(sourcePixelPtr-size_x+1));
	    return Gx+Gy;
	    
	}
	else if (xPos == (size_x-1))
	{
	    Gx=2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr-size_x-1));
	    Gy=2*(*(sourcePixelPtr-size_x))+(*(sourcePixelPtr-size_x-1));
	    return Gx+Gy;
	}
	else
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr-size_x-1)));
	    Gy=2*(*(sourcePixelPtr-size_x))+(*(sourcePixelPtr-size_x+1))+(*(sourcePixelPtr-size_x-1));
	    return abs(Gx)+Gy;
	}
    }
    else
    {
	if (xPos == 0)
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))+(*(sourcePixelPtr+1+size_x));
	    Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x+1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1));
	    return Gx+abs(Gy);
	}
	else if (xPos == (size_x-1))
	{
	    Gx=2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr-size_x-1))+(*(sourcePixelPtr+size_x-1));
	    Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x-1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x-1));
	    return Gx+abs(Gy);
	}
	else
	{
	    Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))-2*(*(sourcePixelPtr-1))-(*(sourcePixelPtr-size_x-1))+(*(sourcePixelPtr+1+size_x))-(*(sourcePixelPtr+size_x-1));
	    Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x+1))-(*(sourcePixelPtr-size_x-1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1))+(*(sourcePixelPtr+size_x-1));
	    return abs(Gx)+abs(Gy);
	}
    }
}

/*
 * This is the function that create the ASCII symbol for every pixel and print it out (without \n or \t)!
 * Arguement List
 **pixel: A pointer points to a pixel of source image;
 **colorScale: The color scale of source image.
 */
char generateASCIISymbol(int scale)
{
    char symbols[] = {' ','\'','=','!','L','T','{','x','e','n','V','G','R','m','Q','@'};
    if (scale>1500)
    {
	return '@';
    }
    else
    {
	return symbols[scale/100];
    }
}

int main()
{
    char current_image=0;
    char runtimeCount=3;
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
	
    while (runtimeCount)
    {
	/* Extract the x and y dimensions of the picture */
	unsigned char x,y;
	unsigned char* shared;
	shared = (unsigned char*) SHARED_ONCHIP_BASE;
	unsigned char size_x, size_y;
	int sobelTemp;
	   
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
	    
	/* Measurement here */
	sram2sm_p3(img_array[current_image]);
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_1);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
	greyscaleGen();
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_2);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,SECTION_3);
	resizeImage();
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_3);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_4);
	size_x = *shared;
	shared = shared + 1;
	size_y = *shared;
	shared = shared + 2;
	   
	#if DEBUG == 1
	printf("\nThe ASCII image is: \n");
	
	for (y=0;y<size_y;y++)
	{
	    for (x=0;x<size_x;x++)
	    {
		sobelTemp = computeSobel(shared,x,y,size_x,size_y);
		printf("%c",generateASCIISymbol(sobelTemp));
		shared++;
	    }
	    printf("\n");
	}
	#else
	unsigned int* sharedInt = (unsigned int*) SHARED_ONCHIP_BASE;
	sharedInt = sharedInt + ((size_x*size_y+3)>>1);
	*sharedInt = size_x;
	sharedInt++;
	*sharedInt = size_y;
	sharedInt= sharedInt+2;
	for (y=0;y<size_y;y++)
	{
	    for (x=0;x<size_x;x++)
	    {
		*sharedInt = computeSobel(shared,x,y,size_x,size_y);
		shared++;
	    }
	}
	#endif
	PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_4);
	    
	    
	    
	/* Print report */
	perf_print_formatted_report
	(PERFORMANCE_COUNTER_0_BASE,
	ALT_CPU_FREQ,        // defined in "system.h"
	4,                   // How many sections to print
	"Copy from SRAM",        // Display-name of section(s).
	"Greyscale",
	"Resizing",
	"Computing Sobels"
	);
	

	runtimeCount--;

	current_image=(current_image+1) % number_of_images;
	delay (300);    
    }
	
    return 0;
}
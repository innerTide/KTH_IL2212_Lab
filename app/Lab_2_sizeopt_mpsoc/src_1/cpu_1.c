#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"


#define DEBUG 0
#define STEP_RESULT 0

#define SIZE_X SHARED_ONCHIP_BASE+8184
#define SIZE_Y SHARED_ONCHIP_BASE+8185
#define COLOR_SCALE SHARED_ONCHIP_BASE+8186

#define RESULT_IMAGE_BASE SHARED_ONCHIP_BASE+5120
#define ASCII_IMAGE_BASE SHARED_ONCHIP_BASE+6144

//Define the semaphores for synchronization
#define SEMAPHORE_0 SHARED_ONCHIP_BASE+8000
#define SEMAPHORE_1 SHARED_ONCHIP_BASE+8001
#define SEMAPHORE_2 SHARED_ONCHIP_BASE+8002
#define SEMAPHORE_3 SHARED_ONCHIP_BASE+8003
#define SEMAPHORE_4 SHARED_ONCHIP_BASE+8004
#define SEMAPHORE_5 SHARED_ONCHIP_BASE+8005

extern void delay (int millisec);

/*
 * This is the function for abs
 */
int abs(int value)
{
    if (value>=0) return value;
    else return -value;
}




int main()
{
    unsigned char symbols[] = {' ','\'','=','!','L','T','{','x','e','n','V','G','R','m','Q','@'};
    #if STEP_RESULT==1
    unsigned char counterGrayscale=0;
    unsigned char counterASCII=0;
    #endif
    while(1)
    {

	/* Extract the x and y dimensions of the picture */
	unsigned char x,y;
	unsigned char* shared;
	unsigned char size_x, size_y;
	
	
	
	/*
	 * This is the section to generate the grayscale image
	 */
	if (IORD_8DIRECT(SEMAPHORE_0,0)&&IORD_8DIRECT(SEMAPHORE_3,0))
	{
	    IOWR_8DIRECT(SEMAPHORE_0,0,0);
	    IOWR_8DIRECT(SEMAPHORE_3,0,0);
	    //Eat the semaphore 0&3
	    
	    unsigned char* greyscalePos;
	    unsigned int temp;
	    shared = (unsigned char*) SHARED_ONCHIP_BASE;
	    greyscalePos = (unsigned char*) RESULT_IMAGE_BASE;
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
	    //End of generation
	    
	    
	    #if STEP_RESULT==1
	    counterGrayscale++;
	    alt_printf("#%x\n",counterGrayscale);
	    alt_putstr("Grayscale:\n");
	    for (y=0 ;y<size_y; y++)
	    {
		for (x=0 ;x<size_x; x++)
		{
		    alt_printf("%x\t",IORD_8DIRECT(RESULT_IMAGE_BASE,size_x*y+x));
		}
		alt_putstr("\n");
	    }
	    #endif
	    //Print out the grayscale result.
	    
	    
	    
	    
	    IOWR_8DIRECT(SEMAPHORE_1,0,1);
	    IOWR_8DIRECT(SEMAPHORE_2,0,1);
	    //Send semaphore 1&2
	}
	
	
	
	/*
	 * This is the section to resize the image and generate the ASCII image
	 */
	if(IORD_8DIRECT(SEMAPHORE_2,0)&&IORD_8DIRECT(SEMAPHORE_5,0))
	{
	    IOWR_8DIRECT(SEMAPHORE_2,0,0);
	    IOWR_8DIRECT(SEMAPHORE_5,0,0);
	    //Eat the semaphore 2&5
	    
	    
	    
	    unsigned char* resizedImagePos = (unsigned char*) RESULT_IMAGE_BASE;;
	    shared =(unsigned char*) RESULT_IMAGE_BASE;
	    int sobelOfPixel,Gx,Gy; 
	    unsigned char *pixel;
	    unsigned char* asciiImage = (unsigned char*) ASCII_IMAGE_BASE;
	    #if DEBUG == 1
	    
	    size_x = IORD_8DIRECT(SIZE_X,0);
	    size_y = IORD_8DIRECT(SIZE_Y,0);
	    /*
	     * This is the part of resizing the images
	     */
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
	    
	    
	    
	    /*
	     * This is the part of generating ASCII image
	     */
	    for (y=1; y<size_y-1; y++)
	    {
		for (x=1; x<size_x-1; x++)
		{
		    pixel=(unsigned char*) RESULT_IMAGE_BASE + (y*size_x+x);
		    Gx=((*(pixel+1))<<1)+(*(pixel+1-size_x))-((*(pixel-1))<<1)-(*(pixel-size_x-1))+(*(pixel+1+size_x))-(*(pixel+size_x-1));
		    Gy=-((*(pixel-size_x))<<1)-(*(pixel-size_x+1))-(*(pixel-size_x-1))+((*(pixel+size_x))<<1)+(*(pixel+size_x+1))+(*(pixel+size_x-1));
		    sobelOfPixel=(abs(Gx)+abs(Gy))/100;
		    if (sobelOfPixel>15)
		    {
			*asciiImage='@';
		    }
		    else
		    {
			*asciiImage=symbols[sobelOfPixel];
		    }
		    pixel++;
		    asciiImage++;
		}
	    }
	    #else
	    /*
	     * 32*32 image resizing
	     */
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
	    
	    
	    
	    /*
	     * 32*32 ASCII image
	     */
	    pixel= (unsigned char*) (RESULT_IMAGE_BASE+15);
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
			*asciiImage=symbols[sobelOfPixel];
		    }
		    pixel++;
		    asciiImage++;
		}
	    }
	    #endif
	    //End of resizing the image
	    
	    
	    
	    
	    IOWR_8DIRECT(SIZE_X,0,size_x);
	    IOWR_8DIRECT(SIZE_Y,0,size_y);
	    //Save the image size
	    
	    
	    
	    
	    #if STEP_RESULT==1
	    counterASCII++;
	    alt_printf("#%x\n",counterASCII);
	    alt_putstr("Resized Image:\n");
	    for (y=0 ;y<size_y; y++)
	    {
		for (x=0 ;x<size_x;x++)
		{
		    alt_printf("%x\t",IORD_8DIRECT(RESULT_IMAGE_BASE,size_x*y+x));
		}
		alt_putstr("\n");
	    }
	    
	    alt_putstr("ASCII Image:\n");
	    for (y=0 ;y<size_y-2; y++)
	    {
		for (x=0 ;x<size_x-2; x++)
		{
		    alt_printf("%c ",IORD_8DIRECT(ASCII_IMAGE_BASE,(size_x-2)*y+x));
		}
		alt_putstr("\n");
	    }
	    #endif
	    //Print out the resized image and ASCII image
	    
	    
	    
	    
	    IOWR_8DIRECT(SEMAPHORE_4,0,1);
	    IOWR_8DIRECT(SEMAPHORE_3,0,1);
	    //Send semaphore 3&4
	    
	}
    }
}

#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"

#define FINISH_INFO 0
#define GRAYSCALE_RESULT 0
#define RESIZE_RESULT 0


#define SIZE_X SHARED_ONCHIP_BASE+8184
#define SIZE_Y SHARED_ONCHIP_BASE+8185
#define COLOR_SCALE SHARED_ONCHIP_BASE+8186

//Need to be modified when CPU is changed

#define RGB_IMAGE_BASE SHARED_ONCHIP_BASE
#define GRAYSCALE_IMAGE_BASE SHARED_ONCHIP_BASE+4096
#define RESIZED_IMAGE_BASE SHARED_ONCHIP_BASE+5120
#define ASCII_IMAGE_BASE SHARED_ONCHIP_BASE+6144

#define LOPE_START 0
#define LOPE_END 8

#define CPU_OFFSET 0
///////////////////////////////////////////////
//Define memory location of Semaphores
#define SEMAPHORE_1 SHARED_ONCHIP_BASE+7168
#define SEMAPHORE_2 SHARED_ONCHIP_BASE+7172
#define SEMAPHORE_3 SHARED_ONCHIP_BASE+7176
#define SEMAPHORE_4 SHARED_ONCHIP_BASE+7180
#define SEMAPHORE_5 SHARED_ONCHIP_BASE+7184
#define SEMAPHORE_6 SHARED_ONCHIP_BASE+7188
#define SEMAPHORE_7 SHARED_ONCHIP_BASE+7196
#define SEMAPHORE_8 SHARED_ONCHIP_BASE+7204


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
    unsigned char x,y; 
    unsigned char* pixel;
    char symbols[] = {' ','\'','=','!','L','T','{','x','e','n','V','G','R','m','Q','@'};
    while (1)
    {
	
	if (IORD_8DIRECT(SEMAPHORE_2,CPU_OFFSET)&&IORD_8DIRECT(SEMAPHORE_4,CPU_OFFSET))
	{
	    IOWR_8DIRECT(SEMAPHORE_2,CPU_OFFSET,0);//Eat the semaphore 2.
	    IOWR_8DIRECT(SEMAPHORE_4,CPU_OFFSET,0);//Eat the semaphore 4.
	    
	    
	    //This is the part of generating gray scale image//
	    unsigned int temp;
	    unsigned char* shared = (unsigned char*) RGB_IMAGE_BASE;
	    unsigned char* greyscalePos = (unsigned char*) GRAYSCALE_IMAGE_BASE;
	    unsigned char x;
	    unsigned char y;
	    for (y=LOPE_START ;y<LOPE_END; y++)
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
	    ////////////////////////////////////////////////////////////////////////////////////////
	    
	    
	    IOWR_8DIRECT(SEMAPHORE_1,CPU_OFFSET,1);
	    IOWR_8DIRECT(SEMAPHORE_3,CPU_OFFSET,1);//The gray scale image has been generated and sends semaphore to CPUs 
	    
	    #if FINISH_INFO==1
	    alt_putstr("CPU_1 GS is finished!\n");
	    #endif
	}
	
	
	
	if (IORD_8DIRECT(SEMAPHORE_3,CPU_OFFSET)&&IORD_8DIRECT(SEMAPHORE_8,CPU_OFFSET))
	{
	    
	    IOWR_8DIRECT(SEMAPHORE_8,CPU_OFFSET,0);//Eat the semaphore 8.
	    IOWR_8DIRECT(SEMAPHORE_3,CPU_OFFSET,0);//Eat the semaphore 3.
	    
	    
	    #if GRAYSCALE_RESULT==1
	    unsigned char* grayscaleTest=(unsigned char*) GRAYSCALE_IMAGE_BASE;
	    unsigned char a,b;
	    alt_putstr("Grayscale image is:\n");
	    for (a=0;a<32;a++)
	    {
		for (b=0;b<32;b++)
		{
		    alt_printf("%x\t",*grayscaleTest);
		    grayscaleTest++;
		}
		alt_putstr("\n");
	    }
	    #endif
	    
	    
	    
	    
	    
	    //This is the part of resizing the image.//
	    unsigned char* shared = (unsigned char*) GRAYSCALE_IMAGE_BASE;
	    unsigned char* resizedImagePos = (unsigned char*) RESIZED_IMAGE_BASE;
	    unsigned char x,y;

	    for (y=LOPE_START; y<LOPE_END; y=y+2)
	    {
		for (x=0; x<32; x=x+2)
		{
		    *resizedImagePos=(*(shared+((y+1)<<5)+x+1)+*(shared+((y+1)<<5)+x)+*(shared+(y<<5)+x+1)+(*(shared+(y<<5)+x)))>>2;

		    resizedImagePos = resizedImagePos + 1;	    
		}
	    }
	    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    
	    
	    IOWR_8DIRECT(SEMAPHORE_7,CPU_OFFSET,1);
	    IOWR_8DIRECT(SEMAPHORE_4,CPU_OFFSET,1);
	    //Image has been resized,and send semaphore 7 & 4.
	    
	    #if FINISH_INFO==1
	    alt_putstr("CPU_1 RS is finished!\n");
	    #endif
	    
	}
	
	
	
	
	if ((IORD_32DIRECT(SEMAPHORE_7,CPU_OFFSET)==0x01010101)&&IORD_8DIRECT(SEMAPHORE_6,CPU_OFFSET))
	{
	    IOWR_8DIRECT(SEMAPHORE_6,CPU_OFFSET,0);//Eat the semaphore 6.
	    IOWR_32DIRECT(SEMAPHORE_7,CPU_OFFSET,0);//Eat the semaphore 7.
	    
	    
	    #if RESIZE_RESULT==1
	    unsigned char* resizeTest=(unsigned char*) RESIZED_IMAGE_BASE;
	    unsigned char a,b;
	    alt_putstr("Resized image is:\n");
	    for (a=0;a<16;a++)
	    {
		for (b=0;b<16;b++)
		{
		    alt_printf("%x\t",*resizeTest);
		    resizeTest++;
		}
		alt_putstr("\n");
	    }
	    #endif
	    
	    // This is the part of computing Sobel.//
	    int sobelOfPixel,Gx,Gy;
	    pixel= (unsigned char*) (RESIZED_IMAGE_BASE+15);
	    unsigned char* asciiImage = (unsigned char*) ASCII_IMAGE_BASE;

	    
	    //This part needs to be modified when CPU is changed! The y loop LOPE_START and LOPE_END.
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
	    //////////////////////////////////////////////////////////////////////////////////////////
	    
	    
	    IOWR_32DIRECT(SEMAPHORE_8,CPU_OFFSET,0x01010101);
	    IOWR_32DIRECT(SEMAPHORE_5,CPU_OFFSET,0x01010101);//ASCII Image has been computed and sends semaphore 4&5 to CPU_0  
	    #if FINISH_INFO==1
	    alt_putstr("CPU_1 Sobel is finished!\n");
	    #endif
	}
    }
}

#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_performance_counter.h"
#include "io.h"
#include "images.h"
#include "stdio.h"

#define TRUE 1
#define IMAGE_RESULT 1

#define RGB_IMAGE_BASE SHARED_ONCHIP_BASE
#define RESIZED_IMAGE_BASE SHARED_ONCHIP_BASE+5120
#define ASCII_IMAGE_BASE SHARED_ONCHIP_BASE+6144
#define SIZE_X SHARED_ONCHIP_BASE+8184
#define SIZE_Y SHARED_ONCHIP_BASE+8185
#define COLOR_SCALE SHARED_ONCHIP_BASE+8186

//Define memory location of Semaphores
#define SEMAPHORE_1 SHARED_ONCHIP_BASE+7168
#define SEMAPHORE_2 SHARED_ONCHIP_BASE+7172
#define SEMAPHORE_3 SHARED_ONCHIP_BASE+7176
#define SEMAPHORE_4 SHARED_ONCHIP_BASE+7180
#define SEMAPHORE_5 SHARED_ONCHIP_BASE+7184
#define SEMAPHORE_6 SHARED_ONCHIP_BASE+7188
#define SEMAPHORE_7 SHARED_ONCHIP_BASE+7196
#define SEMAPHORE_8 SHARED_ONCHIP_BASE+7204

extern void delay (int millisec);


void sram2sm_p3(unsigned char* base)
{
    base=base+3;//Begin to access the RGB value of the image.
    int* source32bit = (int*) base;
    int* destination32bit = (int*) RGB_IMAGE_BASE;
    int i;
    for (i=0;i<768;i++)
    {
	//IOWR_32DIRECT(destination32bit,(i<<2),*(source32bit+i));
    
	*destination32bit++=*source32bit++;
    }
}

void sm2sram(unsigned char* destination)
{
    unsigned char i;
    int* sramPtr32bit = (int*) destination;
    *sramPtr32bit = 0x00100D0D; 
    // The nios ii processor is little endian, so the size_x and size_y need to be stored in lower bits.
    int* smPtr32bit = (int*) ASCII_IMAGE_BASE;
    destination=destination+3;    
    sramPtr32bit = (int*) destination;
    //There are 14*14 pixels in sum and every time we load 4 pixels. So the counter is ended with 49. 
    for (i=0;i<49;i++)
    {
	*sramPtr32bit++ = *smPtr32bit++;
    }
}

int main()
{
    char current_image=0;
    /* Sequence of images for measuring performance */
    char number_of_images=3;
    unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
    unsigned char ASCIIImage[200];
    unsigned char counter=101;
    unsigned int executionTimeCounter=0;
    IOWR_32DIRECT(SEMAPHORE_1,0,0x01010101);
    IOWR_32DIRECT(SEMAPHORE_2,0,0);
    IOWR_32DIRECT(SEMAPHORE_3,0,0);
    IOWR_32DIRECT(SEMAPHORE_4,0,0x01010101);
    IOWR_32DIRECT(SEMAPHORE_5,0,0);
    IOWR_32DIRECT(SEMAPHORE_6,0,0x01010101);
    IOWR_32DIRECT(SEMAPHORE_7,0,0);
    IOWR_32DIRECT(SEMAPHORE_8,0,0x01010101);
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);	 
    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
    while(counter)
    {
	if (IORD_32DIRECT(SEMAPHORE_1,0)==0x01010101)
	{
	    IOWR_32DIRECT(SEMAPHORE_1,0,0);//Eat the semaphore.
	    
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,2);
	    //This is the part of loading images to shared on-chip memory//
	    sram2sm_p3(img_array[current_image]);
	    
	    //////////////////////////////////////////////////////////////
	    PERF_END(PERFORMANCE_COUNTER_0_BASE,2);
	    
	    IOWR_32DIRECT(SEMAPHORE_2,0,0x01010101);
	    //the CPU_0 has load the image to shared memory and sends semaphore 2 to CPU_1-CPU_4
	    current_image=(current_image+1) % number_of_images;
	    
	}
	if (IORD_32DIRECT(SEMAPHORE_5,0)==0x01010101)
	{
	    
	    IOWR_32DIRECT(SEMAPHORE_5,0,0);//Eat the semaphore
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,3);
	    sm2sram(ASCIIImage); 
	    PERF_END(PERFORMANCE_COUNTER_0_BASE,3);
	    PERF_END(PERFORMANCE_COUNTER_0_BASE,1);
	    
	    #if IMAGE_RESULT==1
	    unsigned char* asciiImagePtr = (unsigned char*) ASCII_IMAGE_BASE;
	    int x,y;
	    for(y=0;y<14;y++)
	    {
		for (x=0;x<14;x++)
		{
		    alt_printf("%c ",*asciiImagePtr);
		    asciiImagePtr++;
		}
		alt_putstr("\n");
	    }
	    #endif
	    IOWR_32DIRECT(SEMAPHORE_6,0,0x01010101);//the CPU_0 has saved the ASCII Image into SRAM and sends semaphore 6 to CPU_1-CPU_4
	    perf_print_formatted_report(PERFORMANCE_COUNTER_0_BASE,ALT_CPU_FREQ,3,"One Image","Loading to SM","Saving to SRAM");
	    printf("The real-time throughput is %d.\n",50000000/perf_get_section_time(PERFORMANCE_COUNTER_0_BASE,1));
	    if (counter!=101)
	    {
		executionTimeCounter=perf_get_section_time(PERFORMANCE_COUNTER_0_BASE,1)/50+executionTimeCounter;
	    }
	    if(counter==1)
	    {
		printf("\n\n\nThe total execution time of processing 100 32x32 images is %d us.\nThe averange throughput is %d.",executionTimeCounter,100000000/executionTimeCounter);
	    }
	    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);	 
	    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,1);
	    //delay(500);
	    
	    counter--;
	}
    }
}

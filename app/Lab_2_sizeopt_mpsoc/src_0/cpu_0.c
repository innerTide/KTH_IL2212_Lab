#include "altera_avalon_performance_counter.h"
#include "system.h"
#include "io.h"
#include "images.h"
#include <stdio.h>

#define DEBUG 0

#define SECTION_LOAD_TO_SM 1
#define SECTION_SAVE_TO_SRAM 2
#define TOTAL_EXECUTION_TIME 3


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
	//IOWR_32DIRECT(destination32bit,(i<<2),*(source32bit+i));
	*destination32bit++ = *source32bit++;
    }
    #endif
    
}


/*
 * This is the function that loads ASCII symbol to on-chip memory
 */

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
    for (i=0;i<size_y-2;i++)
    {
	for (j=0;j<size_x-2;j++)
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
	*sramPtr32bit++ = *smPtr32bit++;
    }
    #endif
}

int main()
{
    char current_image=0;
    
    IOWR_8DIRECT(SEMAPHORE_0,0,0);
    IOWR_8DIRECT(SEMAPHORE_1,0,1);
    IOWR_8DIRECT(SEMAPHORE_2,0,0);
    IOWR_8DIRECT(SEMAPHORE_3,0,1);
    IOWR_8DIRECT(SEMAPHORE_4,0,0);
    IOWR_8DIRECT(SEMAPHORE_5,0,1);
    #if DEBUG == 1
    unsigned char counter=10;
    /* Sequence of images for testing if the system functions properly */
    char number_of_images=10;
    unsigned char* img_array[10] = {img1_24_24, img1_32_22, img1_32_32, img1_40_28, img1_40_40, img2_24_24, img2_32_22, img2_32_32, img2_40_28, img2_40_40 };
    /* This is for debugging propose to limit the iteraion.*/
    unsigned char asciiImageStore[400];
    
    #else
    unsigned char counter=0;
    /* Sequence of images for measuring performance */
    char number_of_images=3;
    unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
    unsigned char asciiImageStore[200];
    int executionTimeCounter=0;
    #endif
    
    
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
    PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
    
    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME);
    
    while (counter<101)
    {
	
	
	
	
	
	
	/*
	 * This is the part that loads RGB image to shared memory.
	 */ 
	if (IORD_8DIRECT(SEMAPHORE_1,0))
	{
	    IOWR_8DIRECT(SEMAPHORE_1,0,0);
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, SECTION_LOAD_TO_SM);
	    sram2sm_p3(img_array[current_image]);
	    PERF_END(PERFORMANCE_COUNTER_0_BASE, SECTION_LOAD_TO_SM);
	    IOWR_8DIRECT(SEMAPHORE_0,0,1);
	}


	
	
	
	
	/*
	 * This part is to save ASCII image to SRAM
	 */
	if (IORD_8DIRECT(SEMAPHORE_4,0))
	{
	    IOWR_8DIRECT(SEMAPHORE_4,0,0);
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,SECTION_SAVE_TO_SRAM);
	    sm2sram(asciiImageStore);
	    PERF_END(PERFORMANCE_COUNTER_0_BASE,SECTION_SAVE_TO_SRAM);
	    
	    
	    PERF_END(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME);
	    
	    
	    
	    
	    
	    
	    IOWR_8DIRECT(SEMAPHORE_5,0,1);
	    
	    
	    /* Print report */
	    printf("\n\n\n#%d\n",counter);
	    perf_print_formatted_report
	    (PERFORMANCE_COUNTER_0_BASE,
	     ALT_CPU_FREQ,        // defined in "system.h"
	     3,                   // How many sections to print
	    "Copy from SRAM",        // Display-name of section(s).
	    "Save to SRAM",
	    "Total"
	    );
	    printf("The real-time throughput is %d.\n",50000000/perf_get_section_time(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME));
	    # if DEBUG==0
	    if (counter>0)
	    {
		executionTimeCounter=perf_get_section_time(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME)/50+executionTimeCounter;
	    }
	    if(counter==100)
	    {
		printf("\n\n\nThe total execution time of processing 100 32x32 images is %d us.\nThe averange throughput is %d.",executionTimeCounter,100000000/executionTimeCounter);
	    }
	    #endif
	    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
	    PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
	    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,TOTAL_EXECUTION_TIME);
	    counter=counter+1;
	    current_image=(current_image+1) % number_of_images;
	}
	
	

    }
    
    return 0;
}

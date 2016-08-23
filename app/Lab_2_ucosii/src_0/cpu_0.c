// Skeleton for lab 2
//
// Task 1 writes periodically RGB-images to the shared memory
//
// No guarantees provided - if bugs are detected, report them in the Issue tracker of the github repository!

#include <stdio.h>
#include "altera_avalon_performance_counter.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"
#include "system.h"
#include "io.h"
#include "images.h"

#define DEBUG 0

#define HW_TIMER_PERIOD 100 /* 100ms */

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    StartTask_Stack[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define STARTTASK_PRIO      1
#define TASK1_PRIORITY      10

/* Definition of Task Periods (ms) */
#define TASK1_PERIOD 5000

#define SECTION_1 1
#define SECTION_2 2
#define SECTION_3 3
#define SECTION_4 4

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
	    //*greyscalePos=0.3125*(*shared);
            shared = shared + 1;
            temp=((*shared)<<3)+(*shared)+temp;
            //*greyscalePos=(*shared)*0.5625+*greyscalePos;
	    shared = shared + 1;
            temp=((*shared)<<1)+temp;
	    //*greyscalePos=(*shared)*0.125+*greyscalePos;
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
        }
        else if (xPos == (size_x-1))
        {
            Gx=-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr+size_x-1)));
            Gy=2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x-1));
        }
        else
        {
            Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1+size_x))-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr+size_x-1)));
            Gy=2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1))+(*(sourcePixelPtr+size_x-1));
        }
    }
    else if (yPos == (size_y-1))
    {
        if (xPos == 0)
        {
            Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x));
            Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x+1));
        }
        else if (xPos == (size_x-1))
        {
            Gx=-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr-size_x-1)));
            Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x-1));
        }
        else
        {
            Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))-(2*(*(sourcePixelPtr-1))+(*(sourcePixelPtr-size_x-1)));
            Gy=-(2*(*(sourcePixelPtr-size_x))+(*(sourcePixelPtr-size_x+1))+(*(sourcePixelPtr-size_x-1)));
        }
    }
    else
    {
        if (xPos == 0)
        {
            Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))+(*(sourcePixelPtr+1+size_x));
            Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x+1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1));
        }
        else if (xPos == (size_x-1))
        {
            Gx=-2*(*(sourcePixelPtr-1))-(*(sourcePixelPtr-size_x-1))-(*(sourcePixelPtr+size_x-1));
            Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x-1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x-1));
        }
        else
        {
            Gx=2*(*(sourcePixelPtr+1))+(*(sourcePixelPtr+1-size_x))-2*(*(sourcePixelPtr-1))-(*(sourcePixelPtr-size_x-1))+(*(sourcePixelPtr+1+size_x))-(*(sourcePixelPtr+size_x-1));
            Gy=-2*(*(sourcePixelPtr-size_x))-(*(sourcePixelPtr-size_x+1))-(*(sourcePixelPtr-size_x-1))+2*(*(sourcePixelPtr+size_x))+(*(sourcePixelPtr+size_x+1))+(*(sourcePixelPtr+size_x-1));
        }
    }
    return abs(Gx)+abs(Gy);
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
/*
 * Global variables
 */
int delay; // Delay of HW-timer

/*
 * ISR for HW Timer
 */
alt_u32 alarm_handler(void* context)
{
  OSTmrSignal(); /* Signals a 'tick' to the SW timers */

  return delay;
}

// Semaphores
OS_EVENT *Task1TmrSem;

// SW-Timer
OS_TMR *Task1Tmr;

/* Timer Callback Functions */
void Task1TmrCallback (void *ptmr, void *callback_arg){
  OSSemPost(Task1TmrSem);
}

void task1(void* pdata)
{
	INT8U err;
	INT8U value=0;
	char current_image=0;

	#if DEBUG == 1
	/* Sequence of images for testing if the system functions properly */
	char number_of_images=10;
	unsigned char* img_array[10] = {img1_24_24, img1_32_22, img1_32_32, img1_40_28, img1_40_40,
			img2_24_24, img2_32_22, img2_32_32, img2_40_28, img2_40_40 };
	#else
	/* Sequence of images for measuring performance */
	char number_of_images=3;
	unsigned char* img_array[3] = {img1_32_32, img2_32_32, img3_32_32};
	#endif

	while (1)
	{
		/* Extract the x and y dimensions of the picture */
		unsigned char i = *img_array[current_image];
		unsigned char j = *(img_array[current_image]+1);
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
			printf("%d\t",sobelTemp);
			shared++;
		    }
		    printf("\n");
		}
		#else
		unsigned char asciiImage[256];
		for (y=0;y<size_y;y++)
		{
		    for (x=0;x<size_x;x++)
		    {
			sobelTemp = computeSobel(shared,x,y,size_x,size_y);
			if (sobelTemp>1500)
			{
			    asciiImage[size_x*y+x]='@';
			}
			else
			{
			    asciiImage[size_x*y+x]=generateASCIISymbol((sobelTemp)/100);
			}
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
		"Copy to SRAM",        // Display-name of section(s).
		"Greyscale",
		"Resizing",
		"Computing Sobels"
		);
		printf("The real time throughput is %d.\n",ALT_CPU_FREQ/perf_get_total_time(PERFORMANCE_COUNTER_0_BASE));
		/* Just to see that the task compiles correctly */
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,value++);
		
		OSSemPend(Task1TmrSem, 0, &err);

		/* Increment the image pointer */
		current_image=(current_image+1) % number_of_images;

	}
}

void StartTask(void* pdata)
{
  INT8U err;
  void* context;

  static alt_alarm alarm;     /* Is needed for timer ISR function */

  /* Base resolution for SW timer : HW_TIMER_PERIOD ms */
  delay = alt_ticks_per_second() * HW_TIMER_PERIOD / 1000;
  printf("delay in ticks %d\n", delay);

  /*
   * Create Hardware Timer with a period of 'delay'
   */
  if (alt_alarm_start (&alarm,
      delay,
      alarm_handler,
      context) < 0)
      {
          printf("No system clock available!n");
      }

  /*
   * Create and start Software Timer
   */

   //Create Task1 Timer
   Task1Tmr = OSTmrCreate(0, //delay
                            TASK1_PERIOD/HW_TIMER_PERIOD, //period
                            OS_TMR_OPT_PERIODIC,
                            Task1TmrCallback, //OS_TMR_CALLBACK
                            (void *)0,
                            "Task1Tmr",
                            &err);

   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if creation successful
      printf("Task1Tmr created\n");
    }
   }


   /*
    * Start timers
    */

   //start Task1 Timer
   OSTmrStart(Task1Tmr, &err);

   if (DEBUG) {
    if (err == OS_ERR_NONE) { //if start successful
      printf("Task1Tmr started\n");
    }
   }


   /*
   * Creation of Kernel Objects
   */

  Task1TmrSem = OSSemCreate(0);

  /*
   * Create statistics task
   */

  OSStatInit();

  /*
   * Creating Tasks in the system
   */

  err=OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);

  if (DEBUG) {
     if (err == OS_ERR_NONE) { //if start successful
      printf("Task1 created\n");
    }
   }

  printf("All Tasks and Kernel Objects generated!\n");

  /* Task deletes itself */

  OSTaskDel(OS_PRIO_SELF);
}


int main(void) {

  printf("MicroC/OS-II-Vesion: %1.2f\n", (double) OSVersion()/100.0);

  OSTaskCreateExt(
	 StartTask, // Pointer to task code
         NULL,      // Pointer to argument that is
                    // passed to task
         (void *)&StartTask_Stack[TASK_STACKSIZE-1], // Pointer to top
						     // of task stack
         STARTTASK_PRIO,
         STARTTASK_PRIO,
         (void *)&StartTask_Stack[0],
         TASK_STACKSIZE,
         (void *) 0,
         OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

  OSStart();

  return 0;
}

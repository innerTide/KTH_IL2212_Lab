#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_fifo_util.h"
#include "io.h"

#define TRUE 1
#define VALUE  SHARED_ONCHIP_BASE+3

extern void delay (int millisec);

void init_fifo(void)
{
  // initialise fifo_write
  altera_avalon_fifo_init(FIFO_1_IN_CSR_BASE,
                          0, // disable interrupts,
                          2, // almost empty level
                          FIFO_1_IN_FIFO_DEPTH-2); // almost full level
  // initialise fifo_read
  altera_avalon_fifo_init(FIFO_1_OUT_CSR_BASE,
                          0, // disable interrupts,
                          2, // almost empty level
                          FIFO_1_OUT_FIFO_DEPTH-2); // almost full level
}
int main()
{
  int j;
  int switches = 0;
  char value=0;

  printf("Hello from cpu_0!\n");
  init_fifo();

  while (TRUE) {
	 delay(1000);
	 value=IORD_8DIRECT(VALUE,0);
     printf("Reading from shared memory (unsafe): %d\n",value);
	 switches = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
	 IOWR_ALTERA_AVALON_PIO_DATA(LEDS_RED_BASE,switches);	
	 IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,value);

	 for(j=value; j<value+20; j++) {
		printf("\tWriting to FIFO buffer %d\n",j);
		if(altera_avalon_fifo_write_fifo(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, j) != ALTERA_AVALON_FIFO_OK)
		  printf("FAILED to write %1d since FIFO is full\n",j);
		delay(100);
	 }		
  }
  return 0;
}

#include <stdio.h>
#include "system.h"
#include <altera_avalon_mutex.h>
#include "altera_avalon_fifo_util.h"
#include "io.h"

#define TRUE 1
#define FLAG  SHARED_ONCHIP_BASE
#define VALUE SHARED_ONCHIP_BASE+1

unsigned char value;
extern void delay (int millisec);

int main()
{
  printf("Hello from cpu_1!\n");
  alt_mutex_dev* mutex = altera_avalon_mutex_open( "/dev/mutex_0" );

  delay(600);

  while (TRUE){
	 while((altera_avalon_fifo_read_level(FIFO_1_OUT_CSR_BASE)>0)) {
		delay(150);
		int data = altera_avalon_fifo_read_fifo(FIFO_1_OUT_BASE,FIFO_1_OUT_CSR_BASE);
		printf("Reading from FIFO: %1d\n",data);
		if (IORD_8DIRECT(FLAG,0) == 0) {
		  IOWR_8DIRECT(VALUE,0,++data);
		  IOWR_8DIRECT(FLAG,0,1);
		  printf("Writing to shared memory (safe): %d\n",data);
		}
	 }
  }

  return 0;
}

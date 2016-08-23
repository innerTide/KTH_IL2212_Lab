#include <stdio.h>
#include "system.h"
#include <altera_avalon_mutex.h>
#include "io.h"

#define TRUE 1
#define FLAG  SHARED_ONCHIP_BASE+2
#define VALUE SHARED_ONCHIP_BASE+3

unsigned char value;
extern void delay (int millisec);

int main()
{
  printf("Hello from cpu_3!\n");
  alt_mutex_dev* mutex = altera_avalon_mutex_open( "/dev/mutex_1" );

  while(TRUE) {
	altera_avalon_mutex_lock( mutex, 1 );
	if (IORD_8DIRECT(FLAG,0) == 1) {
		value=IORD_8DIRECT(VALUE,0);
		printf("Reading from shared memory (safe): %d\n",value);
		IOWR_8DIRECT(VALUE,0,++value);
		IOWR_8DIRECT(FLAG,0,0);
		printf("\tWriting to shared memory (safe): %d\n",value);
	}
	altera_avalon_mutex_unlock( mutex );
	delay(10);
  }

  return 0;
}

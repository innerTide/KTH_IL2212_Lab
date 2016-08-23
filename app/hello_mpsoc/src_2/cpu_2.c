#include <stdio.h>
#include "system.h"
#include <altera_avalon_mutex.h>
#include "io.h"

#define TRUE 1
#define FLAGP  SHARED_ONCHIP_BASE
#define VALUEP SHARED_ONCHIP_BASE+1
#define FLAGN  SHARED_ONCHIP_BASE+2
#define VALUEN SHARED_ONCHIP_BASE+3

unsigned char value;
extern void delay (int millisec);

int main()
{
  printf("Hello from cpu_2!\n");
  alt_mutex_dev* mutex0 = altera_avalon_mutex_open( "/dev/mutex_0" );
  alt_mutex_dev* mutex1 = altera_avalon_mutex_open( "/dev/mutex_1" );

  while(TRUE) {
	altera_avalon_mutex_lock( mutex0, 1 );
	if (IORD_8DIRECT(FLAGP,0) == 1) {
		value=IORD_8DIRECT(VALUEP,0);
		printf("Reading from shared memory (safe): %d\n",value);
		altera_avalon_mutex_lock( mutex1, 1 );
		while (IORD_8DIRECT(FLAGN,0) == 1) {
			printf("Locked: %d\n", IORD_8DIRECT(FLAGP,0));
			altera_avalon_mutex_unlock( mutex1 );
			delay(10);
			altera_avalon_mutex_lock( mutex1, 1 );
		}
		IOWR_8DIRECT(VALUEN,0,++value);
		IOWR_8DIRECT(FLAGP,0,0);
		IOWR_8DIRECT(FLAGN,0,1);
		altera_avalon_mutex_unlock( mutex1 );
		printf("\tWriting to shared memory (safe): %d\n",value);
	}
	altera_avalon_mutex_unlock( mutex0 );
	delay(10);
  }

  return 0;
}

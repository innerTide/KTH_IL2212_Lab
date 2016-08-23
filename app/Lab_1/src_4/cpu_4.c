#include "sys/alt_stdio.h"
#include <altera_avalon_mutex.h>

unsigned char counter = 0;

int main()
{
  alt_mutex_dev* mutex0 = altera_avalon_mutex_open( "/dev/mutex_0" );
  /* Event loop never exits. */
  while (1)
  {
    altera_avalon_mutex_lock(mutex0,1);
    if (counter<255) 
    {
	alt_printf("%x : Hello cpu_4!\n",counter);
	counter = counter + 1;
    }
    else 
    {
	counter=0;
    }
    altera_avalon_mutex_unlock(mutex0);
  }

  return 0;
}

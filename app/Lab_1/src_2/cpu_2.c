#include "system.h"
#include <altera_avalon_mutex.h>
#include "io.h"
#include "sys/alt_stdio.h"
#include "altera_avalon_fifo_util.h"

extern void delay(int millisec);

void init_avalon_fifo()
{
    altera_avalon_fifo_init(FIFO_0_IN_CSR_BASE,0,2,FIFO_0_IN_FIFO_DEPTH-2);
    altera_avalon_fifo_init(FIFO_0_OUT_CSR_BASE,0,2,FIFO_0_OUT_FIFO_DEPTH-2);
    alt_putstr("FIFO0 is initialized!\n");
}

int value = 0;

int main()
{
  alt_putstr("Hello cpu_2!\n");
  alt_mutex_dev* mutex0 = altera_avalon_mutex_open( "/dev/mutex_0" );
  init_avalon_fifo();
  /* Event loop never exits. */
  while (1)
  {
    altera_avalon_mutex_lock(mutex0,1);
    if (value<256)
    {
        if (altera_avalon_fifo_write_fifo(FIFO_0_IN_BASE, FIFO_0_IN_CSR_BASE, value) == ALTERA_AVALON_FIFO_OK)
        {
            alt_printf("Write value: %x to FIFO 0 successfully!\n",value);
            value++;
        }
        else
        {
            alt_putstr("Fail to write data to FIFO 0, the FIFO is full!\n");
        }
    }
    else
    {
        value=0;
    }
    delay(50);
    altera_avalon_mutex_unlock(mutex0);
  }
  /*{
    altera_avalon_mutex_unlock(mutex0);
    alt_putstr("CPU 2 gets a semaphore of mutex!\n");
  }*/;

  return 0;
}


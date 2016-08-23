#include "system.h"
#include <altera_avalon_mutex.h>
#include "io.h"
#include "sys/alt_stdio.h"
#include "altera_avalon_fifo_util.h"

extern void delay(int millisec);


int main()
{
  alt_putstr("Hello cpu_3!\n");

  /* Event loop never exits. */
  while (1)
  {
    while ((altera_avalon_fifo_read_level(FIFO_0_OUT_CSR_BASE)>0))
    {        
	delay(70);
        int value = altera_avalon_fifo_read_fifo(FIFO_0_OUT_BASE,FIFO_0_OUT_CSR_BASE);
        alt_printf ("Read value : %x from FIFO 0.\n", value);
    }
  }
  return 0;
}

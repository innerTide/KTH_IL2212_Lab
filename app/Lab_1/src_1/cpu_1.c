#include "system.h"
#include <altera_avalon_mutex.h>
#include "io.h"
#include "sys/alt_stdio.h"
#define FLAG  SHARED_ONCHIP_BASE
#define VALUE SHARED_ONCHIP_BASE+1

extern void delay (int millisec);

unsigned char value = 0;

int main()
{
  alt_putstr("Hello cpu_1!\n");
  IOWR_8DIRECT(FLAG,0,0);
  while(1){
    delay(50);
    if (IORD_8DIRECT(FLAG,0)==0)
    {
        IOWR_8DIRECT(VALUE,0,value);
        alt_printf("Save the counter value: %x to shared memory safely!\n", value);
        value++;
        IOWR_8DIRECT(FLAG,0,1);
    }
    if (value>15)
    {
        value=0;
    }
    //altera_avalon_mutex_lock(mutex0,1);
    //delay(25);
    //altera_avalon_mutex_lock(mutex1,1);
  }

  return 0;
}


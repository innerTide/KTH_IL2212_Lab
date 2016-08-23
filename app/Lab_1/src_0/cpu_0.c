#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_fifo_util.h"
#include "altera_avalon_performance_counter.h"
#include "io.h"
#include "sys/alt_stdio.h"

#define FLAG  SHARED_ONCHIP_BASE
#define VALUE SHARED_ONCHIP_BASE+1

extern void delay (int millisec);

int main()
{
  int switches = 0;
  char keys = 0;
  unsigned char numTable[16]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E}; //Look-up table for the 7 segment display
  char name[]="Code Section 1:";
  unsigned char value = 0;
  alt_putstr("Hello from cpu_0!\n");
  //PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
  while (1) {
	 delay(100);
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);	 
	PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,1);

	 switches = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
	 keys = IORD_ALTERA_AVALON_PIO_DATA (BUTTONS_BASE);
	 IOWR_ALTERA_AVALON_PIO_DATA(LEDS_RED_BASE,switches);
	 IOWR_ALTERA_AVALON_PIO_DATA(LEDS_GREEN_BASE,keys|(keys<<4));
	 IOWR_ALTERA_AVALON_PIO_DATA(HEX3_HEX0_BASE,~((numTable[switches&0xf])|(numTable[(switches&0xf0)>>4]<<8)|(numTable[(switches&0xf00)>>8]<<16)|(numTable[(switches&0xf000)>>12]<<24)));
	 if (IORD_8DIRECT(FLAG,0)==1)
	 {
        value=IORD_8DIRECT(VALUE,0);
        IOWR_ALTERA_AVALON_PIO_DATA(HEX7_HEX4_BASE,(~numTable[value])&0xff);
        alt_printf("Read value : %x from shared memory and put it to LED segment!\n",value);
        IOWR_8DIRECT(FLAG,0,0);
	 }
	 else
	 {
        alt_putstr("Fail to read valure from shared memory\n");
	 }
	 PERF_END(PERFORMANCE_COUNTER_0_BASE,1);
	 perf_print_formatted_report(PERFORMANCE_COUNTER_0_BASE,50000000,1,name);
  }
  PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);
  return 0;
}

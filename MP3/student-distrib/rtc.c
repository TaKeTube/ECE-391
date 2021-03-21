#include "rtc.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"

volatile uint32_t rtc_counter;
int32_t rtc_init()
{
	uint8_t prev;
	/* disable all interrupts, including the NMI */
	cli();
	/* disable NMI*/
	prev = inb(RTC_PORT) | 0x80;
	outb(prev, RTC_PORT);

	/* set default frequency*/
	if(rtc_set_fre(Default_Fre))
		return -1;

	outb(RTC_REGB, RTC_PORT);
	prev = inb(RTC_DATA);
	outb(RTC_REGB, RTC_PORT);
	outb(prev | 0x40, RTC_DATA);

	enable_irq(RTC_IRQ);
	enable_irq(SLAVE_IRQ);

	sti();
	prev = inb(0x70) & 0X7F;
	outb(prev, 0x70);

	rtc_counter = 0;
	return 0;
}

int32_t rtc_set_fre(int32_t fre)
{
	int log = 0;
	uint8_t rate;
	uint8_t prev;
	if (fre < RTC_MIN_FRE && fre > RTC_MAX_FRE)
		return -1;
	if (((fre - 1) & fre))
		return -1;

	while (fre >>= 1)
		log += 1;

	rate = 16 - log;
	/* disable all interrupts, including the NMI */
	cli();
	/* disable NMI*/
	prev = inb(RTC_PORT) | 0x80;
	outb(prev, RTC_PORT);

	outb(RTC_REGA, RTC_PORT);
	prev = inb(RTC_DATA);
	outb(RTC_REGA, RTC_PORT);
	outb((prev & 0xF0) | rate, RTC_PORT);

	sti();
	prev = inb(0x70) & 0X7F;
	outb(prev, 0x70);
	return 0;
}

int32_t rtc_wait() {
	uint32_t current_ticks = rtc_counter;
	// spin until interrupt
	while (current_ticks == rtc_counter);
	return 0;
}

int32_t rtc_write_fre(const void* data, int32_t num) {
	int32_t fre;
	// Check for valid values
	if (data == NULL || num <= 0) {
		return -1;
	}
	// Case for nbytes - cast the void appropriately
	switch (num) {
	case 1:
		fre = *((int8_t*)data);
		break;
	case 2:
		fre = *((int16_t*)data);
		break;
	case 4:
		fre = *((int32_t*)data);
		break;
	default:
		return -1;
	}
	return rtc_set_fre(fre);
}
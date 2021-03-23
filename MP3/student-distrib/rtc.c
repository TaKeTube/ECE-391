#include "rtc.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"

/*
 * rtc_init
 * DESCRIPTION: initialize the rtc.
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 * SIDE AFFECTS: none
 */
int32_t rtc_init()
{
    uint8_t prev;
    /* disable all interrupts, including the NMI */
    cli();
    /* disable NMI*/
    prev = inb(RTC_PORT) | 0x80; //0x80 is used to set the first bit to 1
    outb(prev, RTC_PORT);

    /* set default frequency. if fails, return -1*/
    if (rtc_set_fre(Default_Fre))
        return -1;

    /* set the PIE bit in register B to 1. Thus we can enable the interrupt of RTC*/
    outb(RTC_REGB, RTC_PORT); // set port index
    prev = inb(RTC_DATA); // get original status
    outb(RTC_REGB, RTC_PORT); // set again
    outb(prev | 0x40, RTC_DATA); // set corresponding bit

    /* enable corresponding IRQ for PICs*/
    enable_irq(RTC_IRQ);
    enable_irq(SLAVE_IRQ);

    /* enable NMI*/
    prev = inb(RTC_PORT) & 0X7F; //0x7F is used to set the first bit to 0
    outb(prev, RTC_PORT);
    /* enable all interrupt */
    sti();

    return 0;
}
/*
 * rtc_set_fre
 * DESCRIPTION: set the frequency in RTC
 * INPUT: fre: the target frequency
 * OUTPUT: none
 * RETURN: none
 * SIDE AFFECTS: none
 */
int32_t rtc_set_fre(int32_t fre)
{
    int log = 0;
    uint8_t rate;
    uint8_t prev;
    /* check whether the frequency is in vaild range*/
    if (fre < RTC_MIN_FRE || fre > RTC_MAX_FRE)
        return -1;
    /* check whether the frequency is in power of 2*/
    if (((fre - 1) & fre))
        return -1;
    /* get the log2 value of frequency*/
    while (fre >>= 1)
        log += 1;
    /* get the corresponding rate due to the table 3*/
    rate = 16 - log; //16 is the number of bit patterns in Register A0,A1,A2,A3 due to table 3
    /* disable all interrupts, including the NMI */
    cli();
    /* disable NMI*/
    prev = inb(RTC_PORT) | 0x80; //0x80 is used to set the first bit to 1
    outb(prev, RTC_PORT);

    /* set the frequency bits in register A to the rate value*/
    outb(RTC_REGA, RTC_PORT); // set index to register A
    prev = inb(RTC_DATA); // get value
    outb(RTC_REGA, RTC_PORT); // set index again
    outb((prev & 0xF0) | rate, RTC_PORT); // set the corresponding bits (0-3) 

    /* enable the NMI*/
    prev = inb(RTC_PORT) & 0X7F; //0x7F is used to set the first bit to 0
    outb(prev, RTC_PORT);
    /* enable all interrupts*/
    sti();
    return 0;
}

/*
 * rtc_handler
 * DESCRIPTION: the interrupt handler for rtc
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 * SIDEAFFECTS: none
 */
void rtc_handler() {
    /* mask the interrupts*/
    cli();

    /* test RTC*/
    if(TEST_RTC)
        test_interrupts();

    outb(RTC_REGC, RTC_PORT); // select register C
    inb(RTC_DATA); // throw the contents in register C to reset status bits in register C

    /* send EOI to indicate the handler finishes the work*/
    send_eoi(RTC_IRQ);

    /* unmask the interrupts*/
    sti();
}

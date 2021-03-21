#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* ports for RTC*/
#define RTC_PORT 0x70
#define RTC_DATA 0x71
/* default frequency*/
#define Default_Fre 2
/* register addresses for RTC*/
#define RTC_REGA 0x8A
#define RTC_REGB 0x8B
#define RTC_REGC 0x0C
/* frequency range*/
#define RTC_MIN_FRE 2
#define RTC_MAX_FRE 1024

extern int32_t rtc_init();
extern int32_t rtc_set_fre(int32_t fre);
extern void rtc_handler();

#endif // !_RTC_H
#ifndef _RTC_H_
#define _RTC_H_

#include "types.h"

#define RTC_PORT 0x70
#define RTC_DATA 0x71
#define Default_Fre 2
#define RTC_REGA 0x8A
#define RTC_REGB 0x8B
#define RTC_REGC 0x0C
#define RTC_MIN_FRE 2
#define RTC_MAX_FRE 1024
extern int32_t rtc_init();
extern int32_t rtc_set_fre(int32_t fre);
extern void rtc_handler();

#endif // !_RTC_H_

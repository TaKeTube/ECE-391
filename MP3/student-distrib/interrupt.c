#include "lib.h"
#include "interrupt.h"
#include "i8259.h"


// void keyboard_handler(){
//     cli();
//     printf("keyboard pressed.\n");
//     send_eoi(1);
//     sti();
// }

// void rtc_handler(){
//     cli();
//     printf("RTC interrupt.\n");
//     send_eoi(8);
//     sti();
// }

#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "i8259.h"

#define PIT_DATA_PORT   0x43
#define PIT_CMD_PORT    0x40

extern void pit_init();

extern void pit_handler();

extern void process_switch();

#endif
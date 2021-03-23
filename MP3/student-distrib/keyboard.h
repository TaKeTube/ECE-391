#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEY_NUM 60
#define KEYBOARD_PORT 0x60
#define KEYBOARD_IRQ 1

/* init the keyboard by enabling the corresponding irq line */
extern void keyboard_init();
/* keyboard interrupt handler */
extern void keyboard_handler();

#endif


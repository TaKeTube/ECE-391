#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEY_NUM         60
#define KEYBOARD_PORT   0x60
#define KEYBOARD_IRQ    1
#define READ_BUFFER_SIZE 127
#define BACKSPACE	    0x0E
#define TAB			    0x0F
#define ENTER		    0x1C
#define CAPS_LOCK	    0x3A
#define LSHIFT_DOWN	    0x2A
#define LSHIFT_UP	    0xAA
#define RSHIFT_DOWN	    0x36
#define RSHIFT_UP	    0xB6
#define CTRL_DOWN	    0x1D
#define CTRL_UP		    0x9D
#define ALT_DOWN	    0x38
#define ALT_UP		    0xB8


volatile unsigned char read_buffer[READ_BUFFER_SIZE];
volatile int is_ready;


/* init the keyboard by enabling the corresponding irq line */
extern void keyboard_init();
/* keyboard interrupt handler */
extern void keyboard_handler();
extern void print_key(unsigned char scancode);
extern void clr_read_buffer();


#endif

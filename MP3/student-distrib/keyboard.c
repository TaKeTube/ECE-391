#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

// array for basic key inputs
unsigned char key_table[KEY_NUM] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 
    'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'
};

/*
*	init_keyboard
*	Description: initializes the keyboard 
*	inputs:		nothing
*	outputs:	nothing
*	effects:	enables line for keyboard on the master PIC
*/
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ);
}

/*
*	keyboard_handler
*	Description: If a valid key is pressed, the function echo it onto screen.
*	inputs:	 nothing
*	outputs: nothing
*	side effects: echo current pressed key to screen
*/
void keyboard_handler(){
    unsigned char keydata = 0;
    unsigned char key;

    // mask interrupt
    cli();
    
    // wait for interrupt
    while(1){
        if (inb(KEYBOARD_PORT)){
            keydata = inb(KEYBOARD_PORT);
            break;
        }
    }
    
    // check if key is valid
    if (keydata < KEY_NUM){
        key = key_table[keydata];
        putc(key);
    }

    // end interrupt
    send_eoi(KEYBOARD_IRQ);
    // enable interrupt
    sti();
}



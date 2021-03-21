#define KEY_NUM 60
#define KEYBOARD_PORT 0x60
#define KEYBOARD_IRQ 1

extern void init_keyboard();
extern void keyboard_handler();
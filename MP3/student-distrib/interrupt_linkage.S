#define ASM     1
#include "interrupt_linkage.h"

#define pushall     \
    pushl   %fs;    \
    pushl   %es;    \
    pushl   %ds;    \
    pushl   %eax;   \
    pushl   %ebp;   \
    pushl   %edi;   \
    pushl   %esi;   \
    pushl   %edx;   \
    pushl   %ecx;   \
    pushl   %ebx;

#define popall      \
    popl    %ebx;   \
    popl    %ecx;   \
    popl    %edx;   \
    popl    %esi;   \
    popl    %edi;   \
    popl    %ebp;   \
    popl    %eax;   \
    popl    %ds;    \
    popl    %es;    \
    popl    %fs;

.global int_rtc
int_rtc:
    pushall
    call    rtc_handler
    popall
    iret

.global int_keyboard
int_keyboard:
    pushall
    call    keyboard_handler
    popall
    iret


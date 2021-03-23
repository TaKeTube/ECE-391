#ifndef _IDT_H
#define _IDT_H

#define USER_PRIORITY       3
#define KERNEL_PRIORITY     0

/* Initialize IDT */
extern void idt_init();
/* Set interrupt gate in IDT of one interrupt using the address of the interrupt handler */
inline void set_intr_gate(unsigned int n, void *addr);

#endif
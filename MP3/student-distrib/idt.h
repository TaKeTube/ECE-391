#ifndef _IDT_H
#define _IDT_H

#define USER_PRIORITY       3
#define KERNEL_PRIORITY     0

extern void idt_init();
inline void set_intr_gate(unsigned int n, void *addr);

#endif

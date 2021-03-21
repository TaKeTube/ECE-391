/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    /* mask out all interrupts on the PIC */
    outb(INIT_MASK, MASTER_8259_DATA);
    outb(INIT_MASK, SLAVE_8259_DATA);

    /* initialize the PIC with ICW1*/
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    /* initialize the PIC with ICW2*/
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);

    /* initialize the PIC with ICW3*/
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);

    /* initialize the PIC with ICW4*/
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    // Mask all interrupts
    outb(INIT_MASK, MASTER_8259_DATA);
    outb(INIT_MASK, SLAVE_8259_DATA);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    /* range judge*/
    if (irq_num > MAX_IRQ_NUM || irq_num <0)
        return;
    /* check whether it is master IRQ_num, i.e. 0-7*/
    if (irq_num < MAX_MASTER_IRQ_NUM)
    {
        master_mask = inb(MASTER_8259_DATA);
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
        return;
    }
    /* it is slave IRQ_num, i.e. 8-15*/
    slave_mask = inb(SLAVE_8259_DATA);
    slave_mask &= ~(1 << (irq_num - (MAX_MASTER_IRQ_NUM)));
    outb(slave_mask, SLAVE_8259_DATA);
    return;
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    /* range judge*/
    if (irq_num > MAX_IRQ_NUM || irq_num < 0)
        return;
    /* check whether it is master IRQ_num, i.e. 0-7*/
    if (irq_num < MAX_MASTER_IRQ_NUM)
    {
        master_mask = inb(MASTER_8259_DATA);
        master_mask |= 1 << irq_num;
        outb(master_mask, MASTER_8259_DATA);
        return;
    }
    /* it is slave IRQ_num, i.e. 8-15*/
    slave_mask = inb(SLAVE_8259_DATA);
    slave_mask |= 1 << (irq_num - MAX_MASTER_IRQ_NUM);
    outb(slave_mask, SLAVE_8259_DATA);
    return;
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    /* range judge*/
    if (irq_num > MAX_IRQ_NUM || irq_num < 0)
        return;
    /* check whether it is master IRQ_num, i.e. 0-7*/
    if (irq_num < MAX_MASTER_IRQ_NUM)
    {
        outb(EOI | irq_num, MASTER_8259_PORT);
        return;
    }
    /* it is slave IRQ_num, i.e. 8-15*/
    outb(EOI | SLAVE_PORT, MASTER_8259_PORT);
    outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
}

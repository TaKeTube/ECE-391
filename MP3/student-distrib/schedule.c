#include "schedule.h"
#include "terminal.h"
#include "syscall.h"
#include "x86_desc.h"
#include "lib.h"

/* https://wiki.osdev.org/Programmable_Interval_Timer */
void pit_init()
{
    /* sent command to pit */
    outb(PIT_CMD, PIT_CMD_PORT);
    /*sent least significant bits of period */
    outb(PIT_LATCH && PIT_BITMASK, PIT_CHANNEL_0);
    /*sent most significant bits of period */
    outb(PIT_LATCH >> PIT_MSB_OFFSET, PIT_CHANNEL_0);
    /* enable interrupt */
    enable_irq(PIT_IRQ);
    return;
}

void pit_handler()
{
    send_eoi(PIT_IRQ);
    scheduler();
}

void scheduler()
{
    int i;
    pcb_t* curr_pcb = get_pcb_ptr(curr_pid);
    pcb_t* next_pcb;
    uint32_t next_term_id = 0;
    uint32_t curr_process_term_id = curr_pcb->term_id;
    uint32_t next_pid;

    /* get next process's terminal's id */
    for(i = 0; i < TERMINAL_NUM && !terminals[next_term_id].is_running; i++)
        next_term_id = (curr_process_term_id + i)%TERMINAL_NUM;
    
    /* if process does not change, which means there is only one process running, just return */
    if(next_term_id == curr_process_term_id)
        return 0;

    /* get next process's id */
    next_pid = terminals[next_term_id].curr_pid;

    /* set paging */
    set_paging(next_pid);

    /* remap video memory */
    if(next_term_id == curr_term_id)
        vid_remap((uint8_t *)VIDEO);
    else
        vid_remap(terminals[next_term_id].vid_buf);

    /* get next process's pcb */
    next_pcb = get_pcb_ptr(next_pid);

    /* set current fd array */
    cur_fd_array = next_pcb->fd_array;

    /* set tss */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KS_BASE_ADDR - KS_SIZE * next_pid - sizeof(int32_t);

    /* update current pid */
    curr_pid = next_pid;

    /* store current's esp, ebp */
    asm volatile("                                \n\
        movl %%ebp, %0                            \n\
        movl %%esp, %1                            \n\
        "
        : "=r"(curr_pcb->ebp), "=r"(curr_pcb->esp)
        :
    );

    /* get next process's esp, ebp */
    asm volatile("                                \n\
        movl %0, %%ebp                            \n\
        movl %1, %%esp                            \n\
        "
        :
        : "r"(next_pcb->ebp), "r"(next_pcb->esp)
    );
}


#include "lib.h"
#include "exception.h"

void exc_handler(unsigned int vec);

static char* exception_info[EXC_NUM] = {
    "Division by zero",
    "Single-step interrupt",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bounds",
    "Invalid Opcode",
    "Coprocessor not available",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Invalid Task State Segment",
    "Segment not present",
    "Stack Fault",
    "General protection fault",
    "Page fault",
    "reserved",
    "Math Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};

void exc_handler(unsigned int vec){
    printf("EXCEPTION %d:\n", vec);
    printf("%s\n", exception_info[vec]);
    while(1);
}

void exc_divide_error()              {exc_handler(0x00);}
void exc_single_step()               {exc_handler(0x01);}
void exc_nmi()                       {exc_handler(0x02);}
void exc_breakpoint()                {exc_handler(0x03);}
void exc_overflow()                  {exc_handler(0x04);}
void exc_bounds()                    {exc_handler(0x05);}
void exc_invalid_opcode()            {exc_handler(0x06);}
void exc_coprocessor_not_avaliable() {exc_handler(0x07);}
void exc_double_fault()              {exc_handler(0x08);}
void exc_coprocessor_segment_fault() {exc_handler(0x09);}
void exc_invalid_tss()               {exc_handler(0x0A);}
void exc_segment_not_present()       {exc_handler(0x0B);}
void exc_stack_fault()               {exc_handler(0x0C);}
void exc_general_protection_fault()  {exc_handler(0x0D);}
void exc_page_fault()                {exc_handler(0x0E);}
void exc_reserved()                  {exc_handler(0x0F);}
void exc_math_fault()                {exc_handler(0x10);}
void exc_alignment_check()           {exc_handler(0x11);}
void exc_machine_check()             {exc_handler(0x12);}
void exc_simd_floating_point()       {exc_handler(0x13);}

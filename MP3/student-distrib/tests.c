#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "rtc.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* Exception Test
 * 
 * test exception
 * Inputs: None
 * Outputs: throw corresponding exception on the screen
 * Side Effects: None
 * Coverage: Exceptions
 * Files: exception.h/c, idt.h/c
 */
void exception_test(int vec){
    TEST_HEADER;

    int a;
	int val;
    int* pointer;

    switch (vec)
    {
		case 0x00:
		/* divide_error */
			val = 0; // avoid divide by 0 warning
			a = 2/val;
			break;
		/* test exception by calling interrupt directly */
		case 0x01: asm volatile ("int $0x01"); break;
		case 0x02: asm volatile ("int $0x02"); break;
		case 0x03: asm volatile ("int $0x03"); break;
		case 0x04: asm volatile ("int $0x04"); break;
		case 0x05: asm volatile ("int $0x05"); break;
		case 0x06: asm volatile ("int $0x06"); break;
		case 0x07: asm volatile ("int $0x07"); break;
		case 0x08: asm volatile ("int $0x08"); break;
		case 0x09: asm volatile ("int $0x09"); break;
		case 0x0A: asm volatile ("int $0x0A"); break;
		case 0x0B:
		/* page fault */
			pointer = NULL;
			a = *pointer;
			break;
		case 0x0C: asm volatile ("int $0x0C"); break;
		case 0x0D: asm volatile ("int $0x0D"); break;
		case 0x0E: asm volatile ("int $0x0E"); break;
		case 0x0F: asm volatile ("int $0x0F"); break;
		case 0x10: asm volatile ("int $0x10"); break;
		case 0x11: asm volatile ("int $0x11"); break;
		case 0x12: asm volatile ("int $0x12"); break;
		case 0x13: asm volatile ("int $0x13"); break;
		default: break;
    }
}


/* test paging_init */

/* cases involved in testing*/
enum TESTCASES_PAGING
{
	T_VALID, T_NULL_PTR, T_UNPRESENT_PAGE_1, T_UNPRESENT_PAGE_2, T_VIDEO_OVERFLOW, T_VIDEO_UNDERFLOW
};

/* paging_init() Test
 * 
 * Asserts that paging is initialized
 * Inputs: testcase -- which case is to be tested
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: enable paging and init paging
 * Files: paging.h/c
 */
int test_paging_init(int testcase)
{
	/* local variable that attempts to access certain memory location */
	char test;
	switch (testcase)
	{
		/* testcase1, all these operatons should be valid */
		case T_VALID:
			TEST_HEADER;
			/* access the start of video memory */
			test = *((char *) VIDEO);
			/* access an addr inside video memory, 0x0111 are randomly chosen inside video memory */
			test = *((char *) (VIDEO + 0x0111));	
			/* access the last byte in video memory, (VIDEO, VIDEO+0x0FFF) are valid video memory */
			test = *((char *) (VIDEO + 0x1000 - 1));
			/* access the start of kernel 4mB page, 0x00400000 is the start of kernel page */
			test = *((char *) (0x00400000));
			/* 
				access an addr inside kernel page,
				0x00400000 is the start of kernel page,
				0x0111 are randomly chosen inside video memory 
			*/
			test = *((char *) (0x00400000 + 0x0111));
			/* 
				access the last byte in kernel page
				00000000 01 00 0000 0000 = 0x00400 the start of kernel page,
				00000000 01 11 1111 1111 = 0x007FF the end of the kernel page
			*/
			test = *((char *) (0x007FFFFF));
			return PASS;

		/* 
			testcase2, try to deref a NULL pointer 
			should raise a exception, if not the test fails
		*/
		case T_NULL_PTR:
			TEST_HEADER;
			test = *((char*) NULL);
			return FAIL;

		/*
			testcase3, try to access a page that is not present > 8mB
			should raise a exception, if not the test fails
		*/
		case T_UNPRESENT_PAGE_1:
			TEST_HEADER;
			/* 0xFFFF0000 are randomly chosen, just a large memory addr */
			test = *((char *) 0xFFFF0000);
			return FAIL;
		
		/*
			testcase4, try to access a page that is not present in the first page table
			should raise a exception, if not the test fails
		*/
		case T_UNPRESENT_PAGE_2:
			TEST_HEADER;
			/* 0xB5000 are randomly chosen, just another page in the first page table that is not present */
			/* VIDEO = 0xB8000 */
			test = *((char *) (0xB5000));
			return FAIL;

		/*
			testcase5, try to access video memory larger than 4kB
			should raise a exception, if not the test fails
		*/
		case T_VIDEO_OVERFLOW:
			TEST_HEADER;
			/* (VIDEO, VIDEO+0x0FFF) are valid video memory, here we try to access 1 byte further */
			test = *((char *) (VIDEO + 0x1000));
			return FAIL;

		/*
			testcase6, try to access out of video memory 
			should raise a exception, if not the test fails
		*/
		case T_VIDEO_UNDERFLOW:
			TEST_HEADER;
			/* the memory addr exactly 1 byte before video memory begins */
			test = *((char *) (VIDEO - 0x0001));
			return FAIL;
		default:
			break;
	}
	return FAIL;
}

/* test_bad_input Test
 * 
 * test garbage input
 * Inputs: none
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: garbage input
 * Files: i8259.h/c, idt.h/c, rtc.h/c
 */
int test_bad_input(){
	TEST_HEADER;
	
	int result = PASS;
	/* test PIC functions */
	enable_irq(16);
	disable_irq(16);
	send_eoi(16);
	/* test idt functions */
	set_intr_gate(0,NULL);
	set_intr_gate(NUM_VEC,test_interrupts);
	/* test rtc functions */
	if(!rtc_set_fre(2048)){result = FAIL;}
	if(!rtc_set_fre(3)){result = FAIL;}
	if(!rtc_set_fre(1)){result = FAIL;}
	return result;
}

/* terminal driver test*/
int test_terminal(){
	char buffer[128];
	int r = 0, w = 0;
	printf("terminal driver test begins\n");
	while (1)
	{
		r = terminal_read(0, buffer, 128);
		printf("read buf: %d\n", r);
		if(r > 0)
			w = terminal_write(0, buffer, 128);
		printf("read buf: %d, wrtie buf:%d\n", r, w);
		// printf("%d\n", r);
		// printf("%d\n", w);
		if(r != w)
			break;
	}
	return -1;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("test_paging_init", test_paging_init(T_VALID));s
	// test_terminal_read_write();
	// test_terminal_read_write();
	test_terminal();
	// char a[5] = {'a', 's', 's', 's', 's'};
	// char buf[5] = {'b', 'd', 'd', 's', 's'};
	// int i;
	// terminal_write(0, a, 5);
	// // exception_test(0x00);
	// // launch your tests here
	// // terminal_read(0, buf, 5);
	// for (i=0; i<5; i++)
	// {
	// 	printf("%c", buf[i]);
	// }
	// printf(buf);
	// terminal_write(0, buf, 5);
}

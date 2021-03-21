#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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

/* test init paging */

/* should report error */
int deref_NULL_test() {
	TEST_HEADER;
	char a = *((int*) NULL);
	/* fix compilation warning: unused variable */
	a = a;
	return FAIL;
}

/* should report error */
int deref_test_large_location() {
	TEST_HEADER;
	char a = *((int *) 0xFF000000);
	/* fix compilation warning: unused variable */
	a = a;
	return FAIL;
}

int deref_test_video_mem() {
	TEST_HEADER;
	char a = *((int *) VIDEO);
	/* fix compilation warning: unused variable */
	a = a;
	return PASS;
}

int deref_test_video_mem_plus_2kb() {
	TEST_HEADER;
	char a = *((int *) (VIDEO + 0x0800));		/* 0x0800 => 2048B = 2kB */
	/* fix compilation warning: unused variable */
	a = a;
	return PASS;
}

int deref_test_video_mem_end() {
	TEST_HEADER;
	char a = *((char *) (VIDEO + 0x1000 - 1));		/* 0x1000 => 4096B = 4kB */
	/* fix compilation warning: unused variable */
	a = a;
	return PASS;
}

/* should report error */
int deref_test_video_mem_plus_4kb() {
	TEST_HEADER;
	char a = *((int *) (VIDEO + 0x1000));		/* 0x1000 => 4096B = 4kB */
	/* fix compilation warning: unused variable */
	a = a;
	return FAIL;
}

int deref_test_kernel() {
	TEST_HEADER;
	char a = *((int *) (0x00400000 + 0x400));
	/* fix compilation warning: unused variable */
	a = a;
	return PASS;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("deref_test_kernel", deref_test_kernel());
	// launch your tests here
}

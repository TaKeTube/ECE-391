/*
    paging.c

*/

#include "paging.h"
#include "lib.h"

/* 
 * init_paging
 */
void init_paging()
{
    /* init empty page directory and a page table */
    init_page_directory();
    init_page_table();

    /* activate 0-3mB memory with 4kB pages, connect to the page table */
    page_directory[0].p = 1;
    page_directory[0].base_addr = (unsigned int)page_table >> MEM_OFFSET_BITS;

    /* activate 4-7mB memory with a 4mB page */
    page_directory[1].p = 1;
    page_directory[1].ps = 1;
    page_directory[1].base_addr = 0x400; // a magic number

    /* manipulate hardware, enable paging */
    enable_paging();
    /* activate the video memory page */
    activate_video();
}

/* 
 * init_pde
 */
void init_page_directory()
{
    int i;
    for (i = 0; i < NUM_PD_ENTRY; i++)
    {
        page_directory[i].p = 0;
        page_directory[i].r_w = 1;      // always 1
        page_directory[i].u_s = 0;      // ?
        page_directory[i].pwt = 0;      // always 0
        page_directory[i].pcd = 0;      // ?
        page_directory[i].a = 0;        // won't use, does not matter
        page_directory[i].reserved = 0; // set to 0
        page_directory[i].ps = 0;       // 0 for 4kB; 1 for 4mB
        page_directory[i].g = 0;        // ?
        page_directory[i].avail = 0;    // won't use, does not matter
        page_directory[i].base_addr = 0;
    }
}

/* 
 * init_pte
 */
void init_page_table()
{
    int i;
    for (i = 0; i < NUM_PT_ENTRY; i++)
    {
        page_table[i].p = 0;
        page_table[i].r_w = 1;   // always 1
        page_table[i].u_s = 0;   // ?
        page_table[i].pwt = 0;   // always 0
        page_table[i].pcd = 0;   // ?
        page_table[i].a = 0;     // won't use, does not matter
        page_table[i].d = 0;     // set to 0
        page_table[i].pat = 0;   // set to 0
        page_table[i].g = 0;     // ?
        page_table[i].avail = 0; // won't use, does not matter
        page_table[i].base_addr = i;
    }
}


void enable_paging()
{   
    asm volatile(
        // load cr3 base addr 
        "movl $page_directory, %eax;"
        "andl $0xFFFFFC00, %eax;"
        "movl %eax, %cr3;"

        // Enable Mixture of 4kb and 4mb access
        "movl %cr4, %eax;"
        "orl $0x00000010, %eax;"
        "movl %eax, %cr4;"

        // MSE: enable paging
        "movl %cr0, %eax;"
        "orl $0x80000000, %eax;"
        "movl %eax, %cr0;"
    );
}


/*
 * activate_video
 */
void activate_video()
{
    page_table[VIDEO >> MEM_OFFSET_BITS].p = 1;
}
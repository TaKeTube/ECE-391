/*
    paging.h header file.

*/

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define NUM_PD_ENTRY        1024
#define NUM_PT_ENTRY        1024
#define PAGE_SIZE           4096
#define MEM_OFFSET_BITS     12


typedef struct page_dir_entry
{
    uint32_t p              : 1;
    uint32_t r_w            : 1;
    uint32_t u_s            : 1;
    uint32_t pwt            : 1;
    uint32_t pcd            : 1;
    uint32_t a              : 1;
    uint32_t reserved       : 1;
    uint32_t ps             : 1;
    uint32_t g              : 1;
    uint32_t avail          : 3;
    uint32_t base_addr      : 20;
} page_dir_entry_t;

typedef struct page_table_entry
{
    uint32_t p              : 1;
    uint32_t r_w            : 1;
    uint32_t u_s            : 1;
    uint32_t pwt            : 1;
    uint32_t pcd            : 1;
    uint32_t a              : 1;
    uint32_t d              : 1;
    uint32_t pat            : 1;
    uint32_t g              : 1;
    uint32_t avail          : 3;
    uint32_t base_addr      : 20;
} page_table_entry_t;

page_dir_entry_t page_directory[NUM_PD_ENTRY] __attribute__((aligned(PAGE_SIZE)));
page_table_entry_t page_table[NUM_PT_ENTRY] __attribute__((aligned(PAGE_SIZE)));


void init_paging();
void init_page_directory();
void init_page_table(); 
void enable_paging();
void activate_video();


#endif

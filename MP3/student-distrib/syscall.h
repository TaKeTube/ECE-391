#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"

#define MAX_ARG_LEN     128
#define MAX_FILE_NUM    8

typedef struct pcb_t {
    /* file descriptor array */
    file_desc_t fd_array[MAX_FILE_NUM];
    /* process id */
    uint32_t pid;
    uint32_t parent_pid;
    /* arguments for this process */
    uint8_t arg[MAX_ARG_LEN];
} pcb_t;


typedef struct file_desc_t {
    file_op_table_t* op;    /* file operator table */
    uint32_t inode_idx;     /* inode index */
    uint32_t file_offset;   /* offset in current file */
    uint32_t flags;         /* whether this file descriptor is used */
} file_desc_t;


typedef struct file_op_table_t {
    int32_t (*open)  (const char* filename);
    int32_t (*close) (int32_t fd);
    int32_t (*read)  (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, void* buf, int32_t nbytes);
} file_op_table_t;

int32_t open(const char* filename);

int32_t close(int32_t fd);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, void* buf, int32_t nbytes);

int32_t execute(const uint8_t* cmd);

int32_t halt(uint8_t status);

int32_t getargs(uint8_t* buf, int32_t nbytes);

int32_t vidmap();

#endif
#ifndef _IDT_H
#define _IDT_H

#include "types.h"

#define BLOCK_SIZE_BYTE             4096
#define MAX_FILE_NAME_LEN           32
#define DENTRY_RESERVED_BYTE        24
#define BOOT_BLOCK_RESERVED_BYTE    52
#define MAX_DENTRY_NUM              (BLOCK_SIZE_BYTE-64)/64
#define MAX_INODE_DATA_BLOCK_NUM    (BLOCK_SIZE_BYTE-4)/4

typedef struct dentry_t{
    char        file_name[MAX_FILE_NAME_LEN];
    uint32_t    file_type;
    uint32_t    inode_id;
    uint8_t     reserved[DENTRY_RESERVED_BYTE];
} dentry_t;

typedef struct boot_block_t{
    uint32_t    dir_num;
    uint32_t    inode_num;
    uint32_t    data_block_num;
    uint8_t     reserved[BOOT_BLOCK_RESERVED_BYTE];
    dentry_t    dentries[MAX_DENTRY_NUM];
} boot_block_t;

typedef inode_t{
    uint32_t    file_size;  // file length in Byte
    uint32_t    data_block_ids[MAX_INODE_DATA_BLOCK_NUM];
} inode_t;

typedef struct data_block_t{
    uint8_t     data[BLOCK_SIZE_BYTE];
} data_block_t;

int32_t read_entry_by_name(const uint8_t* fname, dentry_t* dentry);
/* read entry in the boot block in order */
int32_t read_entry_by_index(uint32_t id, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, void* buf, int32_t nbytes);
int32_t file_close(int32_t fd);
int32_t file_open(const char* filename);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_close(int32_t fd);
int32_t dir_open(const char* filename);

#endif

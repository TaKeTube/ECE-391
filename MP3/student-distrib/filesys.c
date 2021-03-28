#include "lib.h"
#include "filesys.h"

void* filesys_addr;
boot_block_t* boot_block;
data_block_t* data_block_arr;
inode_t* inode_arr;
inode_t* cur_file_inode_ptr;
int cur_file_inode_idx;
int cur_file_offset;
int cur_dentry_idx;

void filesys_init(void* filesys){
    filesys_addr = filesys;
    boot_block = filesys;
    inode_arr = &((inode_t*)filesys)[1];
    data_block_arr = &((data_block_t*)filesys)[1+boot_block->inode_num];
    cur_file_inode_ptr = NULL;
}

/* Assume that only 32 length file name would not have a '\0' at the end */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int i;                  /* iterated index for dentries in boot block */
    dentry_t* cur_dentry;   /* pointer to current dentry */

    /* sanity check */
    if(fname == NULL || dentry == NULL || strlen((int8_t*)fname) > MAX_FILE_NAME_LEN)
        return -1;
    /* traverse all dentry until finding the corresponding dentry */
    for(i = 0; i < boot_block->dir_num; i++){
        cur_dentry = &(boot_block->dentry_arr[i]);
        /* compare the file name */
        if(!strncmp((int8_t*)fname, (int8_t*)(cur_dentry->file_name), MAX_FILE_NAME_LEN)){
            /* copy the contents */
            *dentry = *cur_dentry;
            /* success, return 0 */
            return 0;
        }
    }
    /* file not found, return -1 */
    return -1;
}

int32_t read_dentry_by_index(uint32_t idx, dentry_t* dentry){
    /* sanity check */
    if(idx >= boot_block->dir_num)
        return -1;
    /* copy the dentry */
    *dentry = boot_block->dentry_arr[idx];
    /* success */
    return 0;
}

int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buf, uint32_t nbytes){
    int read_bytes;
    int cur_block_num;
    int cur_block_idx;
    int cur_block_offset;
    data_block_t* cur_block;
    inode_t* cur_inode = &(inode_arr[inode_idx]);

    /* sanity check */
    if(buf == NULL || buf == NULL || inode_idx >= boot_block->inode_num || offset > BLOCK_SIZE_BYTE)
        return -1;

    cur_block_num = offset/BLOCK_SIZE_BYTE;
    cur_block_idx = cur_inode->data_block_idx[cur_block_num];
    /* sanity check */
    if(cur_block_idx>=boot_block->data_block_num) return -1;
    cur_block = &(data_block_arr[cur_block_idx]);
    cur_block_offset = offset%BLOCK_SIZE_BYTE;

    for(read_bytes = 0; read_bytes < nbytes; read_bytes++){
        if(cur_block_offset >= BLOCK_SIZE_BYTE){
            cur_block_idx = cur_inode->data_block_idx[++cur_block_num];
            /* sanity check */
            if(cur_block_idx>=boot_block->data_block_num) return -1;
            cur_block = &(data_block_arr[cur_block_idx]);
            cur_block_offset = 0;
        }
        /* if at the end of file, stop reading */
        if(offset++ >= cur_inode->file_size)
            break;
        *(buf++) = cur_block->data[cur_block_offset++];
    }
    /* return the number of bytes read */
    return read_bytes;
}

int32_t file_open(const char* filename){
    dentry_t dentry;    /* temp dentry variable */

    /* read dentry by filename & sanity check */
    if(read_dentry_by_name((uint8_t*)filename, &dentry) != 0 || dentry.file_type != FILE_TYPE || dentry.inode_id >= boot_block->inode_num)
        return -1;
    /* set the global variable (pointer to the current opened file's inode) */
    cur_file_inode_idx = dentry.inode_id;
    cur_file_inode_ptr = &(inode_arr[dentry.inode_id]);
    cur_file_offset = 0;

    /* success, return 0 */
    return 0;
}

int32_t file_close(int32_t fd){
    /* clear the pointer to the current opened file's inode */
    cur_file_inode_ptr = NULL;
    return 0;
}

int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    int read_bytes;
    /* check whether the file is open */
    if(cur_file_inode_ptr == NULL)
        return -1;
    /* read data from file */
    if((read_bytes = read_data(cur_file_inode_idx, cur_file_offset, buf, nbytes))==-1)
        return -1;
    /* update offset if success */
    cur_file_offset += read_bytes;
    /* return the number of bytes read */
    return read_bytes;
}

int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    int read_len;
    char* filename;
    char strbuf[MAX_FILE_NAME_LEN];
    int i;

    /* sanity check */
    if(buf == NULL || cur_dentry_idx >= boot_block->dir_num)
        return -1;

    /* if the file name is bigger than 32, just copy 32 char */
    read_len = MAX_FILE_NAME_LEN<nbytes?MAX_FILE_NAME_LEN:nbytes;
    filename = boot_block->dentry_arr[cur_dentry_idx++].file_name;

    /* copy the filename to the buffer*/
    for(i = 0; i < read_len; i++)
        strbuf[i] = filename[i];

    strncpy((int8_t*)buf, (int8_t*)strbuf, read_len);

    return 0;
}

int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t dir_open(const char* filename){
    return -1;
}

int32_t dir_close(int32_t fd){
    return -1;
}


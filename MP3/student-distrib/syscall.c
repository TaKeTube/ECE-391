#include "syscall.h"


int32_t open(const char* fname){
    dentry_t dentry;
    int fd;

    /* sanity check */
    if(fname == NULL || cur_fd_array == NULL)
        return -1;

    /* find unused file descriptor */
    for(fd = FDA_FILE_START_IDX; fd < MAX_FILE_NUM; fd++){
        if(cur_fd_array[fd].flags == 0)
            break;
    }

    /* fail if reach the max file number or could not find the file */
    if(fd >= MAX_FILE_NUM || read_dentry_by_name(fname, &dentry) != 0)
        return -1;

    /* set the file operator table pointer */
    cur_fd_array[fd].op = &file_op_table_arr[dentry.file_type];

    /* call the coorsponding routine, if fail return -1 */
    if(cur_fd_array[fd].op->open(fd)==-1)
        return -1;

    /* if success, set the file descriptor */
    cur_fd_array[fd].inode_idx = (dentry.file_type == RTC_TYPE) ? -1 : dentry.inode_idx;
    cur_fd_array[fd].file_offset = 0;
    cur_fd_array[fd].flags = 1;

    return 0;
}

int32_t close(int32_t fd){
    int ret;    /* return value of perticular close function */
    
    /* sanity check */
    if(fd < FDA_FILE_START_IDX || fd > MAX_FILE_NUM || cur_fd_array == NULL || cur_fd_array[fd].flags == 0)
        return -1;

    /* close the file and clear the file descriptor */
    if((ret = cur_fd_array[fd].op->close(fd)) == 0){
        /* if successfully closed, clear the file descriptor */
        cur_fd_array[fd].op = NULL;
        cur_fd_array[fd].inode_idx = -1;
        cur_fd_array[fd].file_offset = 0;
        cur_fd_array[fd].flags = 0;
    }

    return ret;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes){
    /* sanity check */
    if(fd < FDA_FILE_START_IDX || fd > MAX_FILE_NUM || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == 0)
        return -1;

    /* call corresponding read function */
    return cur_fd_array[fd].op->read(fd, buf, nbytes);
}

int32_t write(int32_t fd, void* buf, int32_t nbytes){
    /* sanity check */
    if(fd < FDA_FILE_START_IDX || fd > MAX_FILE_NUM || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == 0)
        return -1;

    /* call corresponding write function */
    return cur_fd_array[fd].op->write(fd, buf, nbytes);
}

int32_t halt(uint8_t status){

}

int32_t execute(const uint8_t* cmd){

}

int32_t getargs(uint8_t* buf, int32_t nbytes){

}
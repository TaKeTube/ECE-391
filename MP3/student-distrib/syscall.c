#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "x86_desc.h"
#include "rtc.h"
#include "filesys.h"
#include "terminal.h"

/* file operation table array */
static file_op_table_t file_op_table_arr[FILE_TYPE_NUM];
/* process id array */
static uint32_t pid_array[NUM_PROCESS] = {0};
uint32_t pid = 0;
uint32_t parent_pid = 0;

/*
 * halt
 * DESCRIPTION: terminates a process, returning the specified value to its parent process
 * INPUT: status - halt status
 * OUTPUT: different halt return value to indicate halt status
 * RETURN: 256 if halt by exception, otherwise status
 * SIDE AFFECTS: context switch & PCB cleared & current file descriptor array changed
 */
int32_t halt(uint8_t status)
{
    int fd;                             /* file descriptor array index */
    uint16_t retval;                    /* return value */
    pcb_t *pcb_ptr, *parent_pcb_ptr;    /* pcb pointer */

    /* get current pcb */
    pcb_ptr = (pcb_t*)(KS_BASE_ADDR - KS_SIZE*(pid+1));

    /* clear pid */
    pid_array[pcb_ptr->pid] = 0;

    /* get parent pcb */
    parent_pcb_ptr = (pcb_t*)(KS_BASE_ADDR - KS_SIZE*((pcb_ptr->parent_pid)+1));

    /* clear fd array, close any relevant files */
    for(fd = FDA_FILE_START_IDX; fd < MAX_FILE_NUM; fd++){
        if(cur_fd_array[fd].flags)
            close(fd);
    }
    /* clear stdin fd */
    cur_fd_array[0].op = NULL;
    cur_fd_array[0].flags = FD_FLAG_FREE;
    /* clear stdout fd */
    cur_fd_array[0].op = NULL;
    cur_fd_array[0].flags = FD_FLAG_FREE;

    /* restore parent fd array */
    cur_fd_array = parent_pcb_ptr->fd_array;

    /* restore parent paging */
    set_paging(parent_pcb_ptr->pid);

    /* restore tss data */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KS_BASE_ADDR - KS_SIZE*parent_pcb_ptr->pid - sizeof(int32_t);

    /* if it is the base shell, restart it */
    if((pid == 0) && (pid == parent_pid)){
        clear();
        execute((uint8_t*)"shell");
    }

    /* update pid */
    pid = parent_pcb_ptr->pid;
    parent_pid = parent_pcb_ptr->parent_pid;

    /* decide return value according to the halt status */
    retval = (status == HALT_EXCEPTION) ? HALT_EXCEPTION_RETVAL : (uint16_t)status;

    /* halt */
    asm volatile("              \n\
        movl    %0, %%esp       \n\
        movl    %1, %%ebp       \n\
        xorl    %%eax, %%eax    \n\
        movw    %2, %%ax        \n\
        leave                   \n\
        ret                     \n\
        "
        : 
        : "r" (pcb_ptr->parent_esp), "r"(pcb_ptr->parent_ebp), "r"(retval)
        : "esp", "ebp", "eax"
    );

    /* never reach here */
    return -1;
}

/*
 * execute
 * DESCRIPTION: system call execute, attempts to load and execute a new program, 
 *              handing off the processor to the new program until it terminates.
 * INPUT: cmd -- pointer pointes to the command string
 * OUTPUT: none
 * RETURN: 0 for success, -1 for fail
 * SIDE AFFECTS: context switch & PCB added & current file descriptor array changed
 */
int32_t execute(const uint8_t *cmd)
{
    /* parsed command and argument */
    uint8_t command[MAX_CMD_LEN];
    uint8_t argument[MAX_ARG_LEN];
    /* file excitability check buffer */
    uint8_t check_buffer[CHECK_BUFFER_SIZE];
    /* loop index */
    int i;
    /* start, end of the cmd and arg, used in parser */
    int start, end;
    /* check dentry in executable check */  
    dentry_t check_dentry;
    /* new process id */
    uint32_t new_pid;
    /* pcb pointer */
    pcb_t* pcb_ptr;
    /* EIP and ESP setting */
    uint32_t user_eip, user_esp;

    /* forbid interrupt */
    cli();

    /* ================================= *
     * 1. parse the command and argument *
     * ================================= */

    /* start from position 0 */
    start = 0;
    /* until a non-space char appears */
    while (' ' == cmd[start])   start++;
    /* parse the commands */
    for (i = start; i < strlen((int8_t*)cmd); i++)
    {
        /* if the commands is too long, report an error */
        if(i - start > MAX_CMD_LEN - 1)
        {
            sti();
            return -1;
        }
        /* commands ends if meet a empty char */
        if(cmd[i] == ' ')   break;
        /* store the parsed cmd into a buffer */
        command[i-start] = cmd[i];
    }
    /* end of the command */
    command[i-start] = '\0';
    /* skip all the empty char between command and argument */
    start = i;
    while (' ' == cmd[start]) start++;
    /* get the length of argument */
    end = start;
    while (cmd[end] != '\0' && cmd[end] != ' ' && cmd[end] != '\n') end++;
    /* also stores the argument into a buffer */
    for (i = start; i < end; i++)
        argument[i-start] = cmd[i];
    /* end of the argument */
    argument[i-start] = '\0';

    /* =========================== *
     * 2. check file executability *
     * =========================== */

    /* is a file in the fs? */
    if(0 != read_dentry_by_name(command, &check_dentry)){
        sti();
        return -1;
    }

    /* is valid exectuable? */
    read_data(check_dentry.inode_idx, 0, check_buffer, CHECK_BUFFER_SIZE);
    /* check the magic number of the excutable file: 0x7F, E, L, F */
    if(check_buffer[0]!=0x7f && check_buffer[1]!='E' && check_buffer[2]!='L' && check_buffer[3]!= 'F')
    {
        sti();
        return -1;
    }


    /* ============= *
     * 3. set paging *
     * ============= */

    /* get new process id */
    if ((new_pid = get_new_pid()) != -1)
    {
        pid = new_pid;
        set_paging(pid); 
    }else{
        /* Current number of running process exceeds */
        sti();
        return HALT_SPECIAL;
    }

    /* ==================== *
     * 4. load user program *
     * ==================== */
    if(read_data(check_dentry.inode_idx, 0, (uint8_t*)PROGRAM_VIRTUAL_ADDR, get_file_size(&check_dentry)) == -1){
        sti();
        return -1;
    }

    /* ================= *
     * 5. initialize PCB *
     * ================= */

    /*
     *  pcb struct reference:
     *      typedef struct pcb_t {
     *          file_desc_t fd_array[MAX_FILE_NUM];
     *          uint32_t pid;
     *          uint32_t parent_pid;
     *          uint8_t arg[MAX_ARG_LEN];
     *          uint32_t parent_ebp;
     *          uint32_t parent_esp;
     *          uint32_t subprogram_user_eip;
     *          uint32_t subprogram_user_esp;
     *          uint32_t tss_esp;
     *      } pcb_t; 
     */

    /* get new pcb address */
    pcb_ptr = (pcb_t*)(KS_BASE_ADDR - KS_SIZE*(pid+1));
    /* set process id */
    pcb_ptr->pid = pid;
    
    /* set parent process id */
    if(!pid)
        /* if it is the base shell */
        pcb_ptr->parent_pid = pid;
    else
        pcb_ptr->parent_pid = parent_pid;
    parent_pid = pid;
    
    /* initialize the fd_array */
    /* init all file descriptor */
    for (i = 0; i < MAX_FILE_NUM; i++)
    {
        pcb_ptr->fd_array[i].op = NULL;
        pcb_ptr->fd_array[i].inode_idx = -1;
        pcb_ptr->fd_array[i].file_offset = 0;
        pcb_ptr->fd_array[i].flags = FD_FLAG_FREE;
    }

    /* init stdin */
    pcb_ptr->fd_array[0].op = &file_op_table_arr[STD_TYPE];
    pcb_ptr->fd_array[0].flags = FD_FLAG_BUSY;

    /* init stdout */
    pcb_ptr->fd_array[1].op = &file_op_table_arr[STD_TYPE];
    pcb_ptr->fd_array[1].flags = FD_FLAG_BUSY;

    /* set current fd array */
    cur_fd_array = pcb_ptr->fd_array;

    /* set argument */
    strncpy((int8_t*)pcb_ptr->arg,(int8_t*)argument, MAX_ARG_LEN);

    /* set tss */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KS_BASE_ADDR - KS_SIZE * pid - sizeof(int32_t);

    /* store esp and ebp */
    asm volatile("                                \n\
        movl %%ebp, %0                            \n\
        movl %%esp, %1                            \n\
        "
        : "=r"(pcb_ptr->parent_ebp), "=r"(pcb_ptr->parent_esp)
    );

    /* ================================ *
     * 6.context switch to user progeam *
     * ================================ */

    /* set the address of the first instruction */
    user_eip = *(int32_t*)PROGRAM_START_ADDR;
    user_esp = USER_STACK_ADDR;

    /* enable interrupt */
    sti();

    /* set infomation for IRET to user program space */
    asm volatile ("                                                \n\
        movw    %%cx, %%ds                                         \n\
        pushl   %%ecx                                              \n\
        pushl   %%ebx                                              \n\
        pushfl                                                     \n\
        popl    %%ecx                                              \n\
        orl     $0x0200, %%ecx                                     \n\
        pushl   %%ecx                                              \n\
        pushl   %%edx                                              \n\
        pushl   %%eax                                              \n\
        iret                                                       \n\
        "
        :
        : "a"(user_eip), "b"(user_esp), "c"(USER_DS), "d"(USER_CS)
        : "memory"
    );

    return 0;
}

/*
 * open
 * DESCRIPTION: system call open, would call particular device's open function according to the file type
 * INPUT: filename - name of the file which needs to be open
 * OUTPUT: none
 * RETURN: fd index for success, -1 for fail
 * SIDE AFFECTS: none
 */
int32_t open(const char *fname)
{
    dentry_t dentry;    /* temp dentry for file information */
    int fd;             /* file descriptor array index */

    /* sanity check */
    if (fname == NULL || cur_fd_array == NULL)
        return -1;

    /* find unused file descriptor */
    for (fd = FDA_FILE_START_IDX; fd < MAX_FILE_NUM; fd++)
    {
        if (cur_fd_array[fd].flags == FD_FLAG_FREE)
            break;
    }

    /* fail if reach the max file number or could not find the file */
    if (fd >= MAX_FILE_NUM || read_dentry_by_name((uint8_t*)fname, &dentry) != 0)
        return -1;

    /* set the file operator table pointer */
    cur_fd_array[fd].op = &file_op_table_arr[dentry.file_type];

    /* call the coorsponding routine, if fail return -1 */
    if ((cur_fd_array[fd].op->open(fname)) == -1)
        return -1;

    /* if success, set the file descriptor */
    cur_fd_array[fd].inode_idx = (dentry.file_type == FILE_TYPE) ? dentry.inode_idx : -1;
    cur_fd_array[fd].file_offset = 0;
    cur_fd_array[fd].flags = FD_FLAG_BUSY;

    /* return file descriptor index */
    return fd;
}

/*
 * close
 * DESCRIPTION: system call close, would call particular device's close function according to the file type
 * INPUT: fd -- file descriptor array index of the file to be closed
 * OUTPUT: none
 * RETURN: 0 index for success, -1 for fail
 * SIDE AFFECTS: none
 */
int32_t close(int32_t fd)
{
    int ret; /* return value of perticular close function */

    /* sanity check */
    if (fd < FDA_FILE_START_IDX || fd > MAX_FILE_NUM || cur_fd_array == NULL || cur_fd_array[fd].flags == FD_FLAG_FREE)
        return -1;

    /* close the file and clear the file descriptor */
    if ((ret = cur_fd_array[fd].op->close(fd)) == 0)
    {
        /* if successfully closed, clear the file descriptor */
        cur_fd_array[fd].op = NULL;
        cur_fd_array[fd].inode_idx = -1;
        cur_fd_array[fd].file_offset = 0;
        cur_fd_array[fd].flags = FD_FLAG_FREE;
    }

    return ret;
}

/*
 * read
 * DESCRIPTION: system call read, would call particular device's read function according to the file type
 * INPUT: fd -- file descriptor array index of the file to be read
 *        buf -- buffer to be filled in (can be not used)
 *        nbytes -- number of bytes needed to be read (can be not used)
 * OUTPUT: none
 * RETURN: 0 index for success, -1 for fail
 * SIDE AFFECTS: none
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    /* sanity check */
    if (fd < 0 || fd > MAX_FILE_NUM || fd == FD_STDOUT_IDX || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == FD_FLAG_FREE || cur_fd_array[fd].op == NULL)
        return -1;

    /* call corresponding read function */
    return cur_fd_array[fd].op->read(fd, buf, nbytes);
}

/*
 * write
 * DESCRIPTION: system call write, would call particular device's write function according to the file type
 * INPUT: fd -- file descriptor array index of the file to be write
 *        buf -- not used
 *        nbytes -- not used
 * OUTPUT: none
 * RETURN: 0 index for success, -1 for fail
 * SIDE AFFECTS: none
 */
int32_t write(int32_t fd, void *buf, int32_t nbytes)
{
    /* sanity check */
    if (fd < 0 || fd > MAX_FILE_NUM || fd == FD_STDIN_IDX || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == FD_FLAG_FREE || cur_fd_array[fd].op == NULL)
        return -1;

    /* call corresponding write function */
    return cur_fd_array[fd].op->write(fd, buf, nbytes);
}

/* 
*	getargs
*	Description: get args from command and copy it to buffer
*	Input: 	buf -- destination buffer's pointer
* 			nbytes -- number of bytes to copy
*	Output: 0 for success, -1 for failure
*	Side Effect: copy args to buffer
*/
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    /* get current pcb */
    pcb_t* pcb_ptr;
    pcb_ptr = (pcb_t*)(KS_BASE_ADDR - KS_SIZE*(pid+1));

    /* copy to buffer */
    if (buf == NULL)
        return -1;
    else{
        strncpy((int8_t*)buf, (int8_t*)pcb_ptr->arg, nbytes);
        return 0;
    }
}

/* 
*	vidmap
*	Description: maps video memory to user space
*	Input: 	screen_start -- pointer to user video memory
*	Output: 0 for success, -1 for failure
*/
int32_t vidmap(uint8_t** screen_start)
{
    /* check if the pointer is in user space */
    if (screen_start <= ADDR_128MB || screen_start >= ADDR_132MB)
		return -1;

    *screen_start = ADDR_140MB;

    /* initialize the VIDMAP page */
    page_directory[VIDMAP_OFFSET].p           = 1;    // present
    page_directory[VIDMAP_OFFSET].r_w         = 1;
    page_directory[VIDMAP_OFFSET].u_s         = 1;    // user mode
    page_directory[VIDMAP_OFFSET].base_addr   = (unsigned int)vid_page_table >> MEM_OFFSET_BITS;
    vid_page_table[0].p = 1;    
    vid_page_table[0].r_w = 1;  
    vid_page_table[0].u_s = 1;
    vid_page_table[0].base_addr = VIDEO_ADDR >> MEM_OFFSET_BITS;
    flush_TLB();
    return 0;
}

//9
int32_t set_handler()
{
    return -1;
}

//10
int32_t sigreturn()
{
    return -1;
}

/*
 * get_new_pid
 * DESCRIPTION: get new process id by finding unoccupied position of pid_array
 * INPUT: none
 * OUTPUT: new process id
 * RETURN: new process id for success, -1 for fail
 * SIDE AFFECTS: pid_array corresponding to the pid would be set to busy
 */
uint32_t get_new_pid()
{
    int i;  /* loop index */
    /* traverse pid array to find unoccupied position */
    for (i = 0; i < NUM_PROCESS; i++)
    {
        if (!pid_array[i])
        {
            /* find a empty position, set entry to busy (1) */
            pid_array[i] = 1;
            return i;
        }
    }
    /* Current number of running process exceeds */
    printf("Current number of running process exceeds!\n");
    return -1;
}

/*
 * file_op_table_init
 * DESCRIPTION: initialize file operation table array
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 * SIDE AFFECTS: file operation table initialized
 */
void file_op_table_init()
{
    /* init rtc operation table */
    file_op_table_arr[RTC_TYPE].open  = rtc_open;
    file_op_table_arr[RTC_TYPE].close = rtc_close;
    file_op_table_arr[RTC_TYPE].read  = rtc_read;
    file_op_table_arr[RTC_TYPE].write = rtc_write;

    /* init dir operation table */
    file_op_table_arr[DIR_TYPE].open  = dir_open ;
    file_op_table_arr[DIR_TYPE].close = dir_close;
    file_op_table_arr[DIR_TYPE].read  = dir_read ;
    file_op_table_arr[DIR_TYPE].write = dir_write;

    /* init file operation table */
    file_op_table_arr[FILE_TYPE].open  = file_open;
    file_op_table_arr[FILE_TYPE].close = file_close;
    file_op_table_arr[FILE_TYPE].read  = file_read;
    file_op_table_arr[FILE_TYPE].write = file_write;

    /* init stdin/out (terminal) operation table */
    file_op_table_arr[STD_TYPE].open  = terminal_open;
    file_op_table_arr[STD_TYPE].close = terminal_close;
    file_op_table_arr[STD_TYPE].read  = terminal_read;
    file_op_table_arr[STD_TYPE].write = terminal_write;
}
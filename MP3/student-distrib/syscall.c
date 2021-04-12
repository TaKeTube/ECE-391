#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "x86_desc.h"
#include "rtc.h"
#include "filesys.h"
#include "terminal.h"

static file_op_table_t file_op_table_arr[FILE_TYPE_NUM];
static uint32_t pid_array[NUM_PROCESS] = {0};
uint32_t pid = 0;
uint32_t parent_pid = 0;

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

int32_t execute(const uint8_t *cmd)
{
    /* parsed command and argument */
    uint8_t command[MAX_CMD_LEN];
    uint8_t argument[MAX_ARG_LEN];
    /* read buffer*/
    uint8_t read_buffer[CHECK_BUFFER_SIZE];
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
    cli();

    /* ================================= *
     * 1. parse the command and argument *
     * ================================= */
    start = 0;
    /* until a non-space char appears */
    while (' ' == cmd[start])   start++;

    for (i = start; i < strlen((int8_t*)cmd); i++)
    {
        if(i - start > MAX_CMD_LEN - 1)
        {
            sti();
            return -1;
        }
        if(cmd[i] == ' ')   break;
        command[i-start] = cmd[i];
    }
    command[i-start] = '\0';

    start = i;
    while (' ' == cmd[start])   start++;

    end = start;
    while (cmd[end] != '\0' && cmd[end] != ' ' && cmd[end] != '\n') end++;

    for (i = start; i < end; i++)
        argument[i-start] = cmd[i];
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
    read_data(check_dentry.inode_idx, 0, read_buffer, CHECK_BUFFER_SIZE);
    if(read_buffer[0]!=0x7f && read_buffer[1]!='E' && read_buffer[2]!='L' && read_buffer[3]!= 'F')
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
    read_data(check_dentry.inode_idx, 0, (uint8_t*)PROGRAM_VIRTUAL_ADDR, get_file_size(&check_dentry));

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
    // pcb_ptr->parent_esp0 = (int32_t*)pcb_ptr + KS_SIZE - sizeof(int32_t);

    /* store esp and ebp */
    asm volatile("                                \n\
        movl %%ebp, %0                            \n\
        movl %%esp, %1                            \n\
        "
        : "=r"(pcb_ptr->parent_ebp), "=r"(pcb_ptr->parent_esp)
    );

    /* set the address of the first instruction */
    user_eip = *(int32_t*)PROGRAM_START_ADDR;
    user_esp = USER_STACK_ADDR;

    sti();

    /* modify the EIP and ESP */
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

int32_t open(const char *fname)
{
    dentry_t dentry;
    int fd;

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

    return fd;
}

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

int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    /* sanity check */
    if (fd < 0 || fd > MAX_FILE_NUM || fd == FD_STDOUT_IDX || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == FD_FLAG_FREE || cur_fd_array[fd].op == NULL)
        return -1;

    /* call corresponding read function */
    return cur_fd_array[fd].op->read(fd, buf, nbytes);
}

int32_t write(int32_t fd, void *buf, int32_t nbytes)
{
    /* sanity check */
    if (fd < 0 || fd > MAX_FILE_NUM || fd == FD_STDIN_IDX || buf == NULL || cur_fd_array == NULL || cur_fd_array[fd].flags == FD_FLAG_FREE || cur_fd_array[fd].op == NULL)
        return -1;

    /* call corresponding write function */
    return cur_fd_array[fd].op->write(fd, buf, nbytes);
}


int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    return 0;
}

//8
int32_t vidmap(uint8_t** screen_start)
{
    return 0;
}

//9
int32_t set_handler(int32_t signum, void* handler_address)
{
    return 0;
}

//10
int32_t sigreturn(void)
{
    return 0;
}

uint32_t get_new_pid()
{
    int i;
    for (i = 0; i < NUM_PROCESS; i++)
    {
        if (!pid_array[i])
        {
            pid_array[i] = 1;
            return i;
        }
    }
    printf("Current number of running process exceeds!\n");
    return -1;
}

void file_op_table_init()
{
    file_op_table_arr[RTC_TYPE].open  = rtc_open;
    file_op_table_arr[RTC_TYPE].close = rtc_close;
    file_op_table_arr[RTC_TYPE].read  = rtc_read;
    file_op_table_arr[RTC_TYPE].write = rtc_write;

    file_op_table_arr[DIR_TYPE].open  = dir_open ;
    file_op_table_arr[DIR_TYPE].close = dir_close;
    file_op_table_arr[DIR_TYPE].read  = dir_read ;
    file_op_table_arr[DIR_TYPE].write = dir_write;

    file_op_table_arr[FILE_TYPE].open  = file_open;
    file_op_table_arr[FILE_TYPE].close = file_close;
    file_op_table_arr[FILE_TYPE].read  = file_read;
    file_op_table_arr[FILE_TYPE].write = file_write;

    file_op_table_arr[STD_TYPE].open  = terminal_open;
    file_op_table_arr[STD_TYPE].close = terminal_close;
    file_op_table_arr[STD_TYPE].read  = terminal_read;
    file_op_table_arr[STD_TYPE].write = terminal_write;
}

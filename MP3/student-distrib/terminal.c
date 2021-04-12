/*
    terminal driver
    support the open, close, read and write operations of the terminal
*/

#include "terminal.h"
#include "keyboard.h"
#include "lib.h"


/*
*	terminal_open
*	Description:    open a terminal
*	inputs:         filename, not used
*	outputs:	    nothing
*	effects:	    simply does nothing and returns 0 so far
*/
int32_t terminal_open(const char* filename)
{
    return 0;
}


/*
*	terminal_close
*	Description:    close a terminal
*	inputs:         file descriptor fd
*	outputs:	    nothing
*	effects:	    simply does nothing and returns 0 so far
*/
int32_t terminal_close(int32_t fd)
{
    return 0;
}


/*
*	terminal_read
*	Description:    read the terminal input and store to a buffer 
*	inputs:         fd      -- file descriptor
*                   buf     -- a buffer that holds the terminal input
*                   nbytes  -- the number of bytes to read from keyboard buffer
*	returns:	    the actual number of bytes that are read successfully
*	effects:	    read the keyboard input
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    /* sanity check to see whether the read operation is valid */
    if (NULL == buf || 0 == nbytes)
        return -1;
    
    /* loop index*/
    int i = 0;
    /* a flag to show whether the a \n is met in keyboard read buffer */
    int is_over = 0;
    /* return value, the number of bytes read, init to 0 */
    int ret = 0; 

    /* 
        an infinite loop to wait
        only when the enter pressed would the is_ready flag be set
        then the loop breaks
    */
    while(1)
    {
        if (is_ready == 1)
            break;
    }
    
    /* 
        iterate through the keyboard read buffer
        (i < 127) handles buffer overflow, at most 127 chars are legal to read
    */
    for (i = 0; (i < nbytes) && (i < MAX_TERMINAL_BUF_SIZE-1); i++)
    {
        /* fill the buf with terminal input */
        ((char*)buf)[i] = read_buffer[i];
        
        /* if current char is enter, the input is over */
        if (read_buffer[i] == '\n')
        {
            /* set is_over and change the current, last char in buffer to be \0 */
            is_over= 1;
            ((char*)buf)[i] = '\0';
        }
        /* if the input hasn't over, increment return value */
        if (is_over == 0)
            ret++;
    }
    clr_read_buffer();
    return ret;
}


/*
*	terminal_write
*	Description:    write the corresponding number of bytes of a buffer to the terminal
*	inputs:         fd      -- file descriptor
*                   buf     -- a buffer that holds the chars to write to terminal
*                   nbytes  -- the number of bytes to write from the input buffer
*	returns:	    the actual number of bytes that are write successfully
*   outputs:        the chars in the buffer are outputted to terminal
*	effects:	    write infomation to terminal
*/
int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
{
    /* sanity check to see whether the write operation is valid */
    if (NULL == buf || 0 == nbytes)
        return -1;
        
    /* loop index */
    int i = 0;
    /* return value, the number of bytes written, init to 0 */
    int ret = 0;
    /* iterate through the input buffer */
    for (i = 0; i < nbytes; i++)
    {
        /* write the current char in the buffer to terminal */
        putc(((char*)buf)[i]);
        /* if the current char is not end of the string in buffer, increment ret */
        if (((char*)buf)[i] != '\0')
            ret++;
    }
    /* return the number of bytes written */
    return ret;
}

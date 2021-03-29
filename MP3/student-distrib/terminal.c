/*
    terminal driver
*/

#include "terminal.h"
#include "keyboard.h"
#include "lib.h"


int32_t terminal_open(int32_t fd)
{
    return 0;
}


int32_t terminal_close(int32_t fd)
{
    return 0;
}


int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes)
{
    /* sanity check */
    if (0 == buf || 0 == nbytes)
        return -1;
    
    /* loop index*/
    int i = 0, j = 0, k = 0; 
    while(1)
    {
        if (is_ready == 1)
            break;
    }
    
    for (i = 0; (i < nbytes) && (i < 127); i++)
    {
        buf[i] = read_buffer[i];
        if (read_buffer[i] == '\n')
        {
            k = 1;
            buf[i] = '\0';
        }
        if (k == 0)
            j++;
    }
    clr_read_buffer();
    return j;
}


int32_t terminal_write(int32_t fd, char* buf, int32_t nbytes)
{
    /* sanity check */
    if (0 == buf || 0 == nbytes)
        return -1;
        
    /* loop index */
    int i = 0, j = 0;
    for (i = 0; i < nbytes; i++)
    {
        putc(buf[i]);
        if (buf[i] != '\0')
            j++;
    }
    return j;
}

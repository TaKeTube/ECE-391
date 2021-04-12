/*
    terminal driver header file
*/

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

#define MAX_TERMINAL_BUF_SIZE   128

/* open a terminal */
int32_t terminal_open(const char* filename);

/* close a terminal */
int32_t terminal_close(int32_t fd);

/* read the terminal input and store to a buffer */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* write the corresponding number of bytes of a buffer to the terminal */
int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes);


#endif

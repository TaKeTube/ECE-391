/*
    terminal driver header file
*/

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"


int32_t terminal_open(int32_t fd);

int32_t terminal_close(int32_t fd);

int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, char* buf, int32_t nbytes);


#endif

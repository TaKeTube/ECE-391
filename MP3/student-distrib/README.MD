# BUG-LOG && DEV-LOG

#### 2021/03/20 Guan

- Checkpoint 1 GDT & IDT init done
- seems no bug (currently)

#### 2021/03/20 Xie

+ Checkpoint 1 paging init done, all tests passes.
+ a werid bug in ./paging.c: void enable_paging();

        x86-asm
        // IF WRITE IN-LINE ASM IN THE FOLLOWING WAY,
        // THE PROGRAM WILL CRASH
        asm volatile(
                // Enable Mixture of 4kb and 4mb access
                "movl %cr4, %eax;"
                "orl $0x00000010, %eax;"
                "movl %eax, %cr4;"
        
                // MSE: enable paging; LSE: enable protection mode
                "movl %cr0, %eax;"
                "orl $0x80000000, %eax;"
                "movl %eax, %cr0;"
        
                //  load cr3 base addr
                "movl $page_directory, %eax;"
                "andl $0xFFFFFC00, %eax;"
                "movl %eax, %cr3;"
        );
        
        // BUT IN THIS WAY IT WORKS
        // TO BE MORE SPECIFIC, ANY OTHER ORDER CRASHES
        asm volatile(
                // load cr3 base addr 
                "movl $page_directory, %eax;"
                "andl $0xFFFFFC00, %eax;"
                "movl %eax, %cr3;"
        
                // Enable Mixture of 4kb and 4mb access
                "movl %cr4, %eax;"
                "orl $0x00000010, %eax;"
                "movl %eax, %cr4;"
        
                // MSE: enable paging
                "movl %cr0, %eax;"
                "orl $0x80000000, %eax;"
                "movl %eax, %cr0;"
        );
+ What remains to do: a better interface and wrapper of my test code.

#### 2021/03/20 YY
+ PIC initialization and interface finished. No large bug found.

#### 2021/03/21 Li
+ Checkpoint 1 keyboard initialization done.
+ a bug in ./keyboard.h: #define KEYBOARD_PORT 0x21
        should use 0x60 instead of 0x21, since it's keyboard's port index,
        and 0x21 causes error because it's master PIC port.

#### 2021/03/21 YY
+ rtc initialization and primary handler finished.
+ a bug for rtc handler: after each interrupt, the content in register C 
    should be thrown away by reading the register. 

#### 2021/03/21 Guan

- Exception test finished.

#### 2021/03/22 Guan

- Garbage input test finished.

#### 2021/03/27 Guan

- file system finished, not tested. One thing needs to be noticed is that, index starts from 0, so when we compare index with some max_num, we need to use >= or <=.

#### 2021/03/28 Li

+ Function keys added for keyboard.
+ Implemented read buffer.


#### 2021/03/28 Xie
+ terminal_open, terminal_close, terminal_read and terminal_write finished and debugged but not tested.
+ Modify keyboard driver slightly


#### 2021/03/29 Li

+ Implemented screen scrolling.
+ Fixed a bug that caused backspace to overwrite previous rows.
+ RTC test for rtc_read() and rtc_write finished.

#### 2021/03/29 Xie
+ terminal driver test function done.
+ terminal driver test finished.
+ a worthnoting bug, at first there is not a way to sync between keyboard and the terminal. So when performing terminal_read, nothing can be read and the screen would flash with prompt information of test function. As a solution, I introduce a flag called is_ready to communicate between keyboard and terminal. Only when the enter is pressed would this flag be set, telling the terminal a new read is ready.

#### 2021/03/29 Guan

- file system tested over. Surprisingly, no significant bug. One of the insignificant bugs is, I write an always false sanity check in read data. Other bugs are just syntax error or warning.
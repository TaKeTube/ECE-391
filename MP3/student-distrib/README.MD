# BUG-LOG && DEV-LOG

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

#### 2021/03/21 Li
+ Checkpoint 1 keyboard initialization done.
+ a bug in ./keyboard.h: #define KEYBOARD_PORT 0x21
        should use 0x60 instead of 0x21, since it's keyboard's port index,
        and 0x21 causes error because it's master PIC port.
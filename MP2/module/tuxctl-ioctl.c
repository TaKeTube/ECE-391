/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
    printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define EXTRACT_4BIT(arg, n)    ((arg&(0xF<<(n*4)))>>(n*4))
#define DECIMAL_POINT           0x10
#define BIT_MASK(arg, n)        (arg&(0x1<<n))

static spinlock_t tux_lock = SPIN_LOCK_UNLOCKED;
static unsigned long button_data;
static unsigned long led_data;
static unsigned long ack;
static const unsigned char segments_map[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};

int tux_init(struct tty_struct* tty);
int tux_buttions(struct tty_struct* tty, unsigned long arg);
int tux_set_led(struct tty_struct* tty, unsigned long arg);

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
    unsigned long flags;    /* variable to store EFLAG */
    unsigned char cmds[2];  /* commands need to be send */

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
    switch (a)
    {
    case MTCP_ACK:
        /* device finish the task */
        spin_lock_irqsave(&tux_lock, flags);
        ack = 1;
        spin_unlock_irqrestore(&tux_lock, flags);
        break;
    
    case MTCP_BIOC_EVENT:
        /* | right | left | down | up | c | b | a | start | */
        spin_lock_irqsave(&tux_lock, flags);
        /* extract first 4 bit of byte 1 */
        button_data = EXTRACT_4BIT(b, 0);
        /* extract direction indicators and change the order */
        button_data |= (BIT_MASK(c,3)|(BIT_MASK(c,2)>>1)|(BIT_MASK(c,1)<<1)|BIT_MASK(c,0))<<4;
        /* make it active high */
        button_data ^= 0xFF;
        spin_unlock_irqrestore(&tux_lock, flags);
        // code for debug
        // if(button_data!=0) printk("packet:%x\n",button_data);
        break;
    
    case MTCP_RESET:
        cmds[0] = MTCP_BIOC_ON;
        cmds[1] = MTCP_LED_USR;
        tuxctl_ldisc_put(tty, &cmds[0], 1);
        tuxctl_ldisc_put(tty, &cmds[1], 1);
        /* restore led value */
        tux_set_led(tty, led_data);
        break;
    
    default:
        break;
    }
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
            unsigned cmd, unsigned long arg)
{
    switch (cmd) {
    case TUX_INIT:
        return tux_init(tty);
    case TUX_BUTTONS:
        return tux_buttions(tty, arg);
    case TUX_SET_LED:
        return tux_set_led(tty, arg);
    case TUX_LED_ACK:
        return 0;
    case TUX_LED_REQUEST:
        return 0;
    case TUX_READ_LED:
        return 0;
    default:
        return -EINVAL;
    }
}

int tux_init(struct tty_struct* tty){
    unsigned long flags;    /* variable to store EFLAG */
    unsigned char cmds[2];  /* buffer for commands */

    /* critical section */
    spin_lock_irqsave(&tux_lock, flags);
    /* init button data */
    button_data = 0;
    /* init led data */
    led_data = 0;
    spin_unlock_irqrestore(&tux_lock, flags);
    
    /* 
     * Enable buttion interrupt-on-change & 
     * Put the LED display into user-mode, 
     * 1 byte command
     * if failed, return -EINVAL
     */
    cmds[0] = MTCP_BIOC_ON;
    cmds[1] = MTCP_LED_USR;

    spin_lock_irqsave(&tux_lock, flags);
    /* set device busy */
    ack = 0;
    if(tuxctl_ldisc_put(tty, &cmds[0], 1)||tuxctl_ldisc_put(tty, &cmds[1], 1)){
        ack = 1;
        spin_unlock_irqrestore(&tux_lock, flags);
        return -EINVAL;
    }
    spin_unlock_irqrestore(&tux_lock, flags);
    return 0;
}

int tux_buttions(struct tty_struct* tty, unsigned long arg){
    int failed_byte_num;    /* unsuccessful copied byte */
    unsigned long flags;    /* variable to store EFLAG */

    /* check whether the argument is valid */
    if((unsigned char*)arg == NULL)
        return -EINVAL;

    /* critical section, copy button data to user */
    spin_lock_irqsave(&tux_lock, flags);
    failed_byte_num = copy_to_user((unsigned char*)arg, &button_data, sizeof(unsigned char));
    spin_unlock_irqrestore(&tux_lock, flags);

    // code for debug
    // if(button_data!=0) printk("button:%x\n",button_data);

    /* if failed to copy, return 'bad address', else return 0 */
    return (!failed_byte_num)?0:-EFAULT;
}

int tux_set_led(struct tty_struct* tty, unsigned long arg){
    unsigned long flags;            /* variable to store EFLAG */
    unsigned char led_char;         /* char needed to be displayed */
    unsigned char led_segment;      /* segment value corresponding to the char */
    unsigned char led_mask;         /* Bitmask of which LED's to set */
    unsigned char led_dec_point;    /* specify whether the corresponding decimal points should be turned on */
    unsigned char cmds[6];          /* temp variable to pass command */
    int cmd_num = 0;                /* command index */
    int i;                          /* loop index for each char */

    /* get bit mask 4 bit from arg */
    led_mask = EXTRACT_4BIT(arg, 4);
    /* get decimal point indicator from arg */
    led_dec_point = EXTRACT_4BIT(arg, 6);

    /* set led command */
    cmds[cmd_num++] = MTCP_LED_SET;
    /* set Bitmask of which LED's to set to tux */
    cmds[cmd_num++] = led_mask;

    /* send segment values to tux according to bitmask */
    for (i = 0; i < 4; i++){
        if(led_mask&(0x1<<i)){
            /* get char needed to be displayed */
            led_char = EXTRACT_4BIT(arg, i);
            /* get segment value corresponding to the char value */
            led_segment = segments_map[(int)led_char];
            /* add decimal point if needed */
            if(led_dec_point&(0x1<<i)) led_segment |= DECIMAL_POINT;
            /* save segment values */
            cmds[cmd_num++] = led_segment;
            // debug code
            // printk("segment %d:char %x led_segment %x\n",i,led_char,led_segment);
        }
    }

    /* critical section */
    spin_lock_irqsave(&tux_lock, flags);
    /* if other task does not completes */
    if(!ack){
        spin_unlock_irqrestore(&tux_lock, flags);
        return -EINVAL;
    }
    /* set ack to indicate new task has beginned */
    ack = 0;
    /* send all commands to the device. If failed, clear ack */
    if(tuxctl_ldisc_put(tty, cmds, cmd_num)){
        ack = 1;
        spin_unlock_irqrestore(&tux_lock, flags);
        return -EINVAL;
    }

    // debug code
    // printk("success:%x\n",arg);

    /* record led value for reset */
    led_data = arg;
    spin_unlock_irqrestore(&tux_lock, flags);
    /* success */
    return 0;
}

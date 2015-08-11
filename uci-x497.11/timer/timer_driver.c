/*
 *  kbleds.c - Blink keyboard leds until the module is unloaded.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
//#include <linux/console_struct.h> /* For vc_cons */

MODULE_DESCRIPTION("Timer Driver controlling PC Keyboard LEDs.");
MODULE_AUTHOR("Saleem Yamani");
MODULE_LICENSE("GPL");

struct timer_list my_timer;
struct tty_driver *my_driver;

struct kbled_status {
    char kbled_crt;
    char kbled_prev;
} kbledstatus;

#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

/*
 * Function my_timer_func blinks the keyboard LEDs periodically by invoking
 * command KDSETLED of ioctl() on the keyboard driver.
 * To learn more on virtual terminal ioctl operations, please see file:
 *
 * /usr/src/linux/drivers/char/vt_ioctl.c, function vt_ioctl().
 *
 * The argument to KDSETLED is alternatively set to 7 (thus causing the led
 * mode to be set to LED_SHOW_IOCTL, and all the leds are lit) and to 0xFF
 * (any value above 7 switches back the led mode to LED_SHOW_FLAGS, thus
 * the LEDs reflect the actual keyboard status).
 * To learn more on this, please see file:
 *
 * /usr/src/linux/drivers/char/keyboard.c, function setledstate().
 *
 */


int kbleds_on(struct kbled_status *pstatus)
{
    return (pstatus != NULL &&
           (pstatus->kbled_crt == 0x01 ||
            pstatus->kbled_crt == 0x02 ||
            pstatus->kbled_crt == 0x04));
}

void kbleds_next(struct kbled_status *pstatus)
{
    if (pstatus == NULL)
    {
        return;
    }

    switch(pstatus->kbled_prev) {
    case 0:
        pstatus->kbled_crt = 0x01; /* LED_SCROLLL */
        break;
    case 0x01:
        pstatus->kbled_crt = 0x02; /* LED_NUML */
        break;
    case 0x02:
        pstatus->kbled_crt = 0x04; /* LED_CAPSL */
        break;
    case 0x04:
        pstatus->kbled_crt = 0x01; /* And back to LED_SCROLLL */
        break;
    default:
        break;
    }
}

void kbleds_restore(struct kbled_status *pstatus)
{
    if (pstatus == NULL)
    {
        return;
    }

    pstatus->kbled_prev = pstatus->kbled_crt;
    pstatus->kbled_crt = RESTORE_LEDS;
}

static void my_timer_func(unsigned long ptr)
{
    struct kbled_status *pstatus = (struct kbled_status *) ptr;

    if (kbleds_on(pstatus))
    {
        kbleds_restore(pstatus);
    }
    else
        kbleds_next(pstatus);

    (my_driver->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED,
        pstatus->kbled_crt);

    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
}

/*
  Keyboard LED Init Function
*/
static int __init kbleds_init(void)
{
    int i;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);

    for (i = 0; i < MAX_NR_CONSOLES; i++)
    {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "console[%i/%i] #%i, tty %lx\n", i,
            MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
            (unsigned long)vc_cons[i].d->vc_tty);
    }

    printk(KERN_INFO "kbleds: finished scanning consoles\n");

    my_driver = vc_cons[fg_console].d->vc_tty->driver;

    printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);

    /*
     * Set up the LED blink timer the first time
     */
    init_timer(&my_timer);
    my_timer.function = my_timer_func;
    my_timer.data = (unsigned long)&kbledstatus;
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);

    return 0;
}

static void __exit kbleds_cleanup(void)
{
    printk(KERN_INFO "kbleds: unloading...\n");
    del_timer(&my_timer);
    (my_driver->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, RESTORE_LEDS);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);


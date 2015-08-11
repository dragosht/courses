/*
 *  kbleds.c - Blink keyboard leds until the module is unloaded.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include<asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/spinlock.h>


MODULE_DESCRIPTION("Timer Driver controlling PC Keyboard LEDs.");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define ALL_LEDS_ON     0x07
#define RESTORE_LEDS    0xFF

#define MAGIC 'z'
#define GETDELAY _IOR(MAGIC, 1, int*)
#define SETDELAY _IOW(MAGIC, 2, int*)
#define GETLEDS  _IOR(MAGIC, 3, int*)
#define SETLEDS  _IOW(MAGIC, 4, int*)

struct led_state
{
    unsigned int   led_crt;
    unsigned int   led_buff;
    unsigned int   led_delay;
};

#define KBLED_NAME "kbleds"
#define KBLED_MAJOR 250
#define KBLED_MINOR 0

static dev_t                    dev;
static struct class             *kbleds_class;
static struct class_device      *kbleds_class_device;

struct kbleds_blinker
{
    struct cdev             cdev;
    struct timer_list       timer;
    struct tty_driver      *ttydrv;
    struct led_state        state;
    spinlock_t              spinlock;
};

static struct kbleds_blinker        blinker;


static int leds_on(struct led_state *pstate)
{
    return (pstate != NULL &&
            pstate->led_crt != RESTORE_LEDS &&
            (pstate->led_crt & ALL_LEDS_ON));
}

static void leds_restore(struct led_state *pstate)
{
    if (pstate == NULL)
    {
        return;
    }

    pstate->led_crt = RESTORE_LEDS;
}

static void leds_blink(struct led_state *pstate)
{
    if (pstate == NULL)
    {
        return;
    }

    pstate->led_crt = pstate->led_buff;
}


static void kbleds_timer_func(unsigned long ptr)
{
    struct led_state *pstate = (struct led_state *) ptr;

    spin_lock(&blinker.spinlock);

    if (leds_on(pstate)) {
        leds_restore(pstate);
    } else {
        leds_blink(pstate);
    }

    (blinker.ttydrv->ioctl)(vc_cons[fg_console].d->vc_tty, NULL, KDSETLED,
            pstate->led_crt);

    blinker.timer.expires = jiffies + (pstate->led_delay * HZ / 1000);
    add_timer(&blinker.timer);

    spin_unlock(&blinker.spinlock);
}



static int kbleds_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int kbleds_release(struct inode *inode, struct file *file)
{
    return 0;
}

static loff_t kbleds_lseek(struct file *file, loff_t offset, int whence)
{
    return 0;
}

static ssize_t kbleds_read(struct file *file, char* buff, size_t n, loff_t *offset)
{
    int ret;

    spin_lock(&blinker.spinlock);
    ret = copy_to_user(buff, &blinker.state.led_buff, sizeof(unsigned int));
    spin_unlock(&blinker.spinlock);

    if (ret != 0) {
        return -EFAULT;
    }
    return 0;
}

static ssize_t kbleds_write(struct file *file, const char *buff, size_t n, loff_t *offset)
{
    int ret;

    spin_lock(&blinker.spinlock);
    ret = copy_from_user(&blinker.state.led_buff, buff, sizeof(unsigned int));
    spin_unlock(&blinker.spinlock);

    if (ret != 0) {
        return -EFAULT;
    }
    return 0;
}

static int kbleds_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch(cmd) {
    case GETDELAY:
        printk(KERN_INFO"kbleds: ioctl: GETDELAY\n");
        spin_lock(&blinker.spinlock);
        ret = copy_to_user((char*)arg, (char*) &blinker.state.led_delay, sizeof(unsigned int));
        spin_unlock(&blinker.spinlock);
        if (ret) {
            return -EFAULT;
        }
        break;
    case SETDELAY:
        printk(KERN_INFO"kbleds: ioctl: SETDELAY\n");
        spin_lock(&blinker.spinlock);
        ret = copy_from_user((char*)&blinker.state.led_delay, (char*)arg, sizeof(unsigned int));
        spin_unlock(&blinker.spinlock);
        if (ret) {
            return -EFAULT;
        }

        spin_lock(&blinker.spinlock);
        /* Also reset the timer */
        del_timer(&blinker.timer);
        blinker.timer.function = kbleds_timer_func;
        blinker.timer.data = (unsigned long) &blinker.state;
        blinker.timer.expires = jiffies + (blinker.state.led_delay * HZ / 1000);
        add_timer(&blinker.timer);
        spin_unlock(&blinker.spinlock);
        break;
    case GETLEDS:
        printk(KERN_INFO"kbleds: ioctl: GETLEDS\n");
        spin_lock(&blinker.spinlock);
        ret = copy_to_user((char*)arg, (char*) &blinker.state.led_buff, sizeof(unsigned int));
        spin_unlock(&blinker.spinlock);
        if (ret) {
            return -EFAULT;
        }
        break;
    case SETLEDS:
        printk(KERN_INFO"kbleds: ioctl: SETLEDS\n");
        spin_lock(&blinker.spinlock);
        ret = copy_from_user((char*)&blinker.state.led_buff, (char*)arg, sizeof(unsigned int));
        spin_unlock(&blinker.spinlock);
        if (ret) {
            return -EFAULT;
        }
        break;
    default:
        return -ENOIOCTLCMD;
        break;
    }

    return 0;
}

struct file_operations kbleds_fops =
{
    owner:      THIS_MODULE,
    open:       kbleds_open,
    release:    kbleds_release,
    read:       kbleds_read,
    write:      kbleds_write,
    llseek:     kbleds_lseek,
    ioctl:      kbleds_ioctl
};



static int __init kbleds_init(void)
{
    int ret;
    int result;
    int i;

    dev = MKDEV(KBLED_MAJOR, KBLED_MINOR);
    ret = register_chrdev(dev, KBLED_NAME, &kbleds_fops);

    if (ret == -1) {
        printk(KERN_ALERT"kbleds: Error in registering the module\n");
        return -1;
    }

    spin_lock_init(&blinker.spinlock);
    cdev_init(&blinker.cdev, &kbleds_fops);
    blinker.cdev.owner = THIS_MODULE;

    result = cdev_add(&blinker.cdev, dev, 1);
    if (result) {
        printk(KERN_ERR"kbleds: Error adding kbled_cdev\n");
        unregister_chrdev(KBLED_MAJOR, KBLED_NAME);
        return -1;
    }

    kbleds_class = class_create(THIS_MODULE, KBLED_NAME);
    if (IS_ERR(kbleds_class)) {
        printk(KERN_ERR"kbleds: Error creating kbled class\n");
        result = PTR_ERR(kbleds_class);
        cdev_del(&blinker.cdev);
        unregister_chrdev(KBLED_MAJOR, KBLED_NAME);
        return -1;
    }

    kbleds_class_device = class_device_create(kbleds_class, NULL, dev, NULL, KBLED_NAME);
    if (IS_ERR(kbleds_class_device)) {
        printk(KERN_ERR"kbleds: Error creating kbled class device\n");
        result = PTR_ERR(kbleds_class_device);
        cdev_del(&blinker.cdev);
        class_device_destroy(kbleds_class, dev);
        unregister_chrdev(KBLED_MAJOR, KBLED_NAME);
        return -1;
    }
    printk(KERN_INFO"kbleds: Created kbled class device in sysfs successfully\n");

    for (i = 0; i < MAX_NR_CONSOLES; i++) {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "console[%i/%i] #%i, tty %lx\n", i,
            MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
            (unsigned long)vc_cons[i].d->vc_tty);
    }

    blinker.ttydrv = vc_cons[fg_console].d->vc_tty->driver;

    init_timer(&blinker.timer);
    blinker.timer.function = kbleds_timer_func;
    blinker.timer.data = (unsigned long) &blinker.state;

    blinker.state.led_delay = 1000; /* once per second default */
    blinker.state.led_buff = ALL_LEDS_ON;
    blinker.state.led_crt = RESTORE_LEDS;

    blinker.timer.expires = jiffies + (blinker.state.led_delay * HZ / 1000);
    add_timer(&blinker.timer);

    return 0;
}

static void __exit kbleds_cleanup(void)
{
    printk(KERN_INFO"kbleds: unloading ...\n");
    del_timer(&blinker.timer);
    (blinker.ttydrv->ioctl)(vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, RESTORE_LEDS);

    cdev_del(&blinker.cdev);
    class_device_destroy(kbleds_class, dev);
    class_destroy(kbleds_class);

    unregister_chrdev(KBLED_MAJOR, KBLED_NAME);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);



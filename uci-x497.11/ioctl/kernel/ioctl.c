
 /*
 *  ioctl_kernel_space.c - Hello with IOCTL and timers
 *             IOCTL control to external applications
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


MODULE_DESCRIPTION("Example drive illustrating the use of Timers and IOCTL.");
MODULE_AUTHOR("Saleem Yamani");
MODULE_LICENSE("GPL");

#define MAGIC 'z'
#define GETIOCTL _IOR(MAGIC, 1, int *)
#define SETIOCTL _IOW(MAGIC, 2, int *)

#define HELLOPLUS "helloplus"

struct timer_list my_timer;
struct tty_driver *my_driver;
char   helloplustatus = 0;



#define HELLO_DELAY   (HZ)

#define INITIAL_SECS    10
#define DEV_MAJOR       250
#define DEV_MINOR       5

unsigned int hello_major;
unsigned int hello_minor;
static unsigned char secs2hello;

struct file_operations fops;
struct cdev hello_cdev;
dev_t dev;
struct class *hello_class;
struct class_device *hello_class_device;

/* 
*  Timer Function
*/
static void my_timer_func(unsigned long ptr)
{
    /* Set timer expire value */
    my_timer.expires = jiffies + HELLO_DELAY*secs2hello;

    printk("helloplus: timer %lu %lu\n",jiffies,my_timer.expires);
    
	/* Add the timer */
    add_timer(&my_timer);
}


/* IOCTL 
*  Gives you the ability to send commands outside of the normal file commands 
*/
static int my_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    
	switch(cmd)
    {
        case GETIOCTL: /* Get timer value */
            printk("helloplus: ioctl: GETIOCTL\n");
            ret = copy_to_user((char *)arg, (char *)&secs2hello, 1);
            printk("Get:Seconds to Hello %d %d\n",secs2hello, ret);

            break;
   
        case SETIOCTL: /* Set timer value */
            printk("helloplus: ioctl: SETIOCTL\n");
            ret = copy_from_user((char *)&secs2hello, (char *)arg, 1);
            printk("\nSet:Seconds to Hello %d %d\n",secs2hello, ret);
            
			/* Delete old timer */
			del_timer(&my_timer);

            /* Setup new timer */
			my_timer.function = my_timer_func;
            my_timer.data = (unsigned long)&helloplustatus;
            my_timer.expires = jiffies + HELLO_DELAY*secs2hello;
            
			/* Add new timer */
			add_timer(&my_timer);
            
			break;
    }
    return 0;
}

/* 
*  Open Function
*/
static int my_open(struct inode *inode, struct file *file)
{
    printk("helloplus: open\n");
    return 0;
}

/* 
*  Release Function
*/
static int my_release(struct inode *inode, struct file *file)
{
    printk("helloplus: release\n");
    return 0;
}

/* 
*  Lseek Function
*/
static loff_t my_lseek(struct file *file,loff_t offset,int whence)
{
    printk("helloplus: lseek\n");
    return 0;
}


/* 
*  Read Function
*/
static ssize_t my_read(struct file *file, char *buff, size_t n,loff_t *offset)
{
    int ret;
    int position = (long)*offset;
    printk("helloplus: read %d\n", position);
    ret = copy_to_user(buff,&secs2hello,1);
    return 0;
}

/* 
*  Write Function
*/
static ssize_t my_write(struct file *file, const char *buff, size_t n, loff_t *offset)
{
    int ret;
    int position = (long)*offset;
    printk("helloplus: write %d\n",position);
    ret = copy_from_user(&secs2hello,buff,1);
    return 0;
}


/* 
*  File Operation Structure
*/
struct file_operations hello_fops =
{
    owner:   THIS_MODULE,
    open:    my_open,
    release: my_release,
    read:    my_read,
    write:   my_write,
    llseek:  my_lseek,
    ioctl:   my_ioctl,
};

/* 
*  Init Function
*/
static int __init helloplus_init(void)
{
    int ret;
    int result;

    printk("In init module");

    hello_major = DEV_MAJOR;
    hello_minor = DEV_MINOR;
    secs2hello =  INITIAL_SECS;


    dev = MKDEV(hello_major, hello_minor);
    ret = register_chrdev(dev,HELLOPLUS,&hello_fops);

    printk("The device is registered by Major no: %d\n",ret);
    
	if(ret == -1)
    {
        printk("Error in registering the module\n");
        return -1;
    }

    cdev_init(&hello_cdev, &hello_fops);
    hello_cdev.owner = THIS_MODULE;
    
	result = cdev_add(&hello_cdev, dev, 3);
    
	if (result) {
        printk(KERN_ERR "hello (char driver):" "Could not add hello_cdev.\n");
        /* release the previously allocated resources */
        unregister_chrdev(hello_major,HELLOPLUS);
        return -1;
    }


    printk(KERN_INFO "helloplus: %d\n",__LINE__);



    /* Create an entry (class/directory) in sysfs so that udev can
     * automatically create a device file when this modules gets
     * loaded (and this init function gets called).
     */
    hello_class = class_create(THIS_MODULE, "helloplus");
    
	if (IS_ERR(hello_class)) 
	{
        printk(KERN_ERR "Error creating hello class.\n");
        result = PTR_ERR(hello_class);
        cdev_del(&hello_cdev);
        unregister_chrdev(hello_major,HELLOPLUS);
        return -1;
    }

    hello_class_device = class_device_create(hello_class, NULL, dev, NULL, HELLOPLUS);

    printk(KERN_INFO "helloplus: %d\n",__LINE__);

    if (IS_ERR(hello_class_device)) 
	{
        printk(KERN_ERR "Error creating hello class device.\n");
        result = PTR_ERR(hello_class_device);
        cdev_del(&hello_cdev);
        class_device_destroy(hello_class, dev);
        unregister_chrdev(hello_major,HELLOPLUS);
        return -1;
    }
    printk(KERN_INFO "hello (char driver): Created hello class device in sysfs successfully\n");


    printk(KERN_INFO "helloplus: loading\n");

    /*
     * Set up the LED hello timer the first time
     */
    init_timer(&my_timer);
    my_timer.function = my_timer_func;
    my_timer.data = (unsigned long)&helloplustatus;
    my_timer.expires = jiffies + HELLO_DELAY*secs2hello;
    add_timer(&my_timer);


    printk(KERN_INFO "helloplus: %d\n",__LINE__);


    return 0;
}

/* 
*  Cleanup Function
*/
static void __exit helloplus_cleanup(void)
{
    printk(KERN_INFO "helloplus: unloading...\n");
    del_timer(&my_timer);

    cdev_del(&hello_cdev);
    class_device_destroy(hello_class, dev);
    class_destroy(hello_class);

    unregister_chrdev(hello_major,HELLOPLUS);
    printk("In cleanup module\n");

}





module_init(helloplus_init);
module_exit(helloplus_cleanup);

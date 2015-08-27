#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "../include/trtest.h"


MODULE_DESCRIPTION("tracer test module");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");


#define MODULE_NAME		"trtest"
#define MY_MAJOR		42
#define MY_MINOR		0

struct trtest_device
{
	dev_t		dev;
	struct cdev	cdev;
	char*		buffer;
};

static struct trtest_device trdev;

static int trtest_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int trtest_release(struct inode *inode, struct file *file)
{
	return 0;
}

long trtest_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
	case TRTEST_KMALLOC:
		trdev.buffer = kmalloc(arg, GFP_KERNEL);
		if (!trdev.buffer) {
			return -ENOMEM;
		}
		printk(KERN_DEBUG "trtest: kmalloc() addr: %lx size: %lu\n",
			(unsigned long)trdev.buffer, arg);
		break;
	case TRTEST_KFREE:
		printk(KERN_DEBUG "trtest: kfree() addr: %lx\n",
			(unsigned long)trdev.buffer);
		kfree(trdev.buffer);
		trdev.buffer = NULL;
		break;
	}

	return 0;
}

struct file_operations trtest_fops = {
	owner:			THIS_MODULE,
	open:			trtest_open,
	release:		trtest_release,
	unlocked_ioctl:		trtest_ioctl,
};

static int trtest_init(void)
{
	int err;

	trdev.dev = MKDEV(MY_MAJOR, MY_MINOR);
	err = register_chrdev_region(trdev.dev, 1, MODULE_NAME);
	if (err) {
		printk(KERN_ALERT "Unable to register char device region\n");
		return err;
	}

	cdev_init(&trdev.cdev, &trtest_fops);
	err = cdev_add(&trdev.cdev, trdev.dev, 1);
	if (err) {
		printk(KERN_ALERT "Unable to add char device\n");
		goto err_cdev_add;
	}

	return 0;

err_cdev_add:
	unregister_chrdev_region(trdev.dev, 1);
	return err;
}

static void trtest_exit(void)
{
	cdev_del(&trdev.cdev);
	unregister_chrdev_region(trdev.dev, 1);
}

module_init(trtest_init);
module_exit(trtest_exit);


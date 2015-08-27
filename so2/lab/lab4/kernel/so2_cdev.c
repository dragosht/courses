#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_ALERT

#define MY_MAJOR		42
#define MY_MINOR		0
//#define NUM_MINORS		1
#define NUM_MINORS		4
#define MODULE_NAME		"so2_cdev"
#define MESSAGE			"hello\n"
#define IOCTL_MESSAGE		"Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif

struct so2_cdev_devdata
{
	dev_t			dev;
	struct cdev		cdev;
	atomic_t		opened;
	char			buffer[BUFSIZ];
	int			buffstart;
	int			buffsize;
	wait_queue_head_t	wqueue;
	int			cond;
};

struct so2_cdev_data
{
	dev_t			first;
	struct class		*class;
	struct so2_cdev_devdata	devdata[NUM_MINORS];
};

struct so2_cdev_data data;

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_cdev_devdata *dev;

	dev = container_of(inode->i_cdev, struct so2_cdev_devdata, cdev);
	file->private_data = dev;

	/*
	if (atomic_cmpxchg(&dev->opened, 0, 1)) {
		printk(LOG_LEVEL "so2_cdev: device %d busy\n", MINOR(dev->dev));
		return -EBUSY;
	}
	*/

	/*
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(1000);
	*/

	printk(KERN_DEBUG "so2_cdev: open(): %d\n", MINOR(dev->dev));

	return 0;
}

static int so2_cdev_release(struct inode *inode, struct file *file)
{
	struct so2_cdev_devdata *dev = file->private_data;
	atomic_set(&dev->opened, 0);
	printk(KERN_DEBUG "so2_cdev: release()\n");
	return 0;
}

static ssize_t so2_cdev_read(struct file *file, char *buff, size_t count, loff_t *offset)
{
	int result = 0;
	struct so2_cdev_devdata *dev = file->private_data;

	if (*offset > BUFSIZ) {
		goto out;
	}
	if (*offset + count > BUFSIZ) {
		count = BUFSIZ - *offset;
	}

	if (copy_to_user(buff, &dev->buffer[0] + *offset, count)) {
		printk(LOG_LEVEL "Unable to read device buffer\n");
		return -EFAULT;
	}
	result = count;
	*offset += count;
out:
	return result;
}

static ssize_t so2_cdev_write(struct file *file, const char *buff, size_t count, loff_t *offset)
{
	int result = 0;

	struct so2_cdev_devdata *dev = file->private_data;

	if (*offset > BUFSIZ) {
		goto out;
	}
	if (*offset + count > BUFSIZ) {
		count = BUFSIZ - *offset;
	}

	if (copy_from_user(&dev->buffer[0] + *offset, buff, count)) {
		printk(LOG_LEVEL "Unable to write device buffer\n");
		return -EFAULT;
	}
	result = count;
	*offset += count;

out:
	return result;
}

long so2_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long result = 0;
	struct so2_cdev_devdata *dev = file->private_data;

	switch (cmd) {
	case MY_IOCTL_PRINT:
		printk(LOG_LEVEL "%s\n", IOCTL_MESSAGE);
		break;
	case MY_IOCTL_SET_BUFFER:
		if (copy_from_user(dev->buffer, (char*)arg, BUFFER_SIZE)) {
			printk(LOG_LEVEL "so2_cdev (ioctl): Unable to set buffer\n");
			return -EFAULT;
		}
		dev->buffstart = 0;
		dev->buffsize = strlen(dev->buffer);
		break;
	case MY_IOCTL_GET_BUFFER:
		if (copy_to_user((char*)arg, dev->buffer, BUFFER_SIZE)) {
			printk(LOG_LEVEL "so2_cdev (ioctl): Unable to get buffer\n");
			return -EFAULT;
		}
		break;
	case MY_IOCTL_DOWN:
		wait_event_interruptible(dev->wqueue, dev->cond != 0);
		dev->cond = 0;
		break;
	case MY_IOCTL_UP:
		dev->cond = 1;
		wake_up_interruptible(&dev->wqueue);
		break;
	default:
		return -EINVAL;
		break;
	}
	return result;
}


struct file_operations so2_cdev_fops = {
	owner:			THIS_MODULE,
	open:			so2_cdev_open,
	release:		so2_cdev_release,
	read:			so2_cdev_read,
	write:			so2_cdev_write,
	llseek:			NULL,
	unlocked_ioctl:		so2_cdev_ioctl,
};

static int so2_cdev_init(void)
{
	int err;
	int i;

	data.first = MKDEV(MY_MAJOR, MY_MINOR);
	err = register_chrdev_region(data.first, NUM_MINORS, MODULE_NAME);
	if (err) {
		printk(LOG_LEVEL "Unable to register char device region\n");
		goto err_register_chrdev;
	}

	/* For dynamic region allocation */
	/*
	err = alloc_chrdev_region(&data.dev, 0, NUM_MINORS, MODULE_NAME);
	if (err) {
		printk(LOG_LEVEL "Unable to allocate chrdev region\n");
		goto err_register_chrdev;
	}
	printk(KERN_DEBUG "Allocated chrdev: %d %d\n", MAJOR(data.dev), MINOR(data.dev));
	*/

	data.class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(data.class)) {
		printk(LOG_LEVEL "Unable to create so2_cdev class\n");
		goto err_class_create;
	}

	for (i = 0; i < NUM_MINORS; i++) {
		data.devdata[i].dev = MKDEV(MY_MAJOR, MY_MINOR + i);
		if (!device_create(data.class, NULL, data.devdata[i].dev,
				NULL, MODULE_NAME"%d", i)) {
			printk(LOG_LEVEL "Unable to create device %d\n", i);
			goto err_device_create;
		}
	}

	for (i = 0; i < NUM_MINORS; i++) {
		atomic_set(&data.devdata[i].opened, 0);
		memset(data.devdata[i].buffer, 0, BUFSIZ);
		data.devdata[i].buffstart = 0;
		data.devdata[i].buffsize = 0;
		/* strcpy(data.devdata[i].buffer, MESSAGE); */
		init_waitqueue_head(&data.devdata[i].wqueue);
		data.devdata[i].cond = 0;

		cdev_init(&data.devdata[i].cdev, &so2_cdev_fops);
		err = cdev_add(&data.devdata[i].cdev, data.devdata[i].dev, 1);
		if (err) {
			printk(LOG_LEVEL "Unable to add char device: %d\n", i);
			goto err_cdev_add;
		}
	}

	return 0;

err_cdev_add:
	i--;
	while (i >= 0) {
		cdev_del(&data.devdata[i].cdev);
	}
	i = NUM_MINORS;
err_device_create:
	i--;
	while (i >= 0) {
		device_destroy(data.class, data.devdata[i].dev);
	}
	class_destroy(data.class);
err_class_create:
	unregister_chrdev_region(data.first, NUM_MINORS);
err_register_chrdev:
	return err;
}

static void so2_cdev_exit(void)
{
	int i;
	for (i = 0; i < NUM_MINORS; i++) {
		cdev_del(&data.devdata[i].cdev);
	}
	for (i = 0; i < NUM_MINORS; i++) {
		device_destroy(data.class, data.devdata[i].dev);
	}
	class_destroy(data.class);
	unregister_chrdev_region(data.first, NUM_MINORS);
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);


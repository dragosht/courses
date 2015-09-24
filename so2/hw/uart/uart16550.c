#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>


#include "uart16550.h"

MODULE_DESCRIPTION("UART16550 device driver");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define MODULE_NAME "uart16550"

#define IRQ_COM1 4
#define IRQ_COM2 3

#define COM1_BASE_ADDR 0x3f8
#define COM2_BASE_ADDR 0x2f8

#define REG_THR 0 /* Transmit Holding Register */
#define REG_RHR 0 /* Receive Holding Register */
#define REG_IER 1 /* Interrupt Enable Register */
#define REG_FCR 2 /* FIFO control Register */
#define REG_ISR 2 /* Interrupt Status Register */
#define REG_LCR 3 /* Line Control Register */
#define REG_MCR 4 /* Modem Control Register */
#define REG_LSR 5 /* Line Status Register */
#define REG_MSR 6 /* Modem Status Register */
#define REG_SPR 7 /* Scratchpad Register */

static int major = 42;
static int option = OPTION_BOTH;

module_param(major, int, S_IRUSR);
//MODULE_PARAM_DESC(param_major, "major");
module_param(option, int, S_IRUSR);
//MODULE_PARAM_DESC(param_option, "option");

struct uart16550_devdata
{
	dev_t           dev;
	struct cdev     cdev;
};

struct uart16550_devdata data[MAX_NUMBER_DEVICES];

static int uart16550_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int uart16550_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t uart16550_read(struct file *file, char __user *buff, size_t count, loff_t *offset)
{
	return 0;
}

static ssize_t uart16550_write(struct file *file, const char __user *buff, size_t count, loff_t *offset)
{
	return 0;
}

static long uart16550_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

irqreturn_t uart16550_interrupt_handle(int irq_no, void *dev_id)
{
	return IRQ_NONE;
}


struct file_operations uart16550_fops = {
	owner:			THIS_MODULE,
	open:			uart16550_open,
	release:		uart16550_release,
	read:			uart16550_read,
	write:			uart16550_write,
	llseek:			NULL,
	unlocked_ioctl:		uart16550_ioctl,

};

static int uart16550_dev_init(int i)
{
	int err;
	data[i].dev = MKDEV(major, i);
	cdev_init(&data[i].cdev, &uart16550_fops);
	err = cdev_add(&data[i].cdev, data[i].dev, 1);
	if (err) {
		printk(KERN_ALERT "Unable to add char device: %d\n", i);
		goto err_cdev_add;
	}
	return 0;
}

static int uart16550_init(void)
{
	int err, i;

	printk("[uart16550] major: %d\n", major);
	printk("[uart16550] option: %d\n", option);

	err = register_chrdev_region(data[0].dev, MAX_NUMBER_DEVICES, "uart");
	if (err) {
		printk(KERN_ALERT "Unable to register char device region\n");
		goto err_register_chrdev;
	}

	for (i = 0; i < MAX_NUMBER_DEVICES; i++) {
		uart16550_dev_init(i);
		if (option & UART16550_COM1_SELECTED) {
			uart16550_dev_init(0);
		}
		if (option & UART16550_COM2_SELECTED) {
			data[1].dev = MKDEV(major, 1);
		}
	}

	return 0;

err_cdev_add:
	i--;
	while (i >= 0) {
		cdev_del(&data[i].cdev);
	}
err_register_chrdev:
	return err;
}

static void uart16550_exit(void)
{
	int i;

	cdev_del(&data[i].cdev);
	unregister_chrdev_region(data[0].dev, MAX_NUMBER_DEVICES);
}

module_init(uart16550_init);
module_exit(uart16550_exit);


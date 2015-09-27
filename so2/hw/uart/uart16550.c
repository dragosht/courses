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
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>


#include "uart16550.h"

MODULE_DESCRIPTION("UART16550 device driver");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define MODULE_NAME "uart16550"

#define IRQ_COM1 4
#define IRQ_COM2 3

#define COM1_BASE_ADDR 0x3f8
#define COM2_BASE_ADDR 0x2f8
#define NPORTS 8

#define REG_THR_OFFSET 0 /* Transmit Holding Register */
#define REG_RHR_OFFSET 0 /* Receive Holding Register */
#define REG_DLL_OFFSET 0 /* Divisor Latch Low */
#define REG_IER_OFFSET 1 /* Interrupt Enable Register */
#define REG_DLH_OFFSET 1 /* Divisor Latch High */
#define REG_FCR_OFFSET 2 /* FIFO control Register */
#define REG_ISR_OFFSET 2 /* Interrupt Status Register */
#define REG_LCR_OFFSET 3 /* Line Control Register */
#define REG_MCR_OFFSET 4 /* Modem Control Register */
#define REG_LSR_OFFSET 5 /* Line Status Register */
#define REG_MSR_OFFSET 6 /* Modem Status Register */
#define REG_SPR_OFFSET 7 /* Scratchpad Register */

static int major = 42;
static int option = OPTION_BOTH;

module_param(major, int, S_IRUSR);
MODULE_PARM_DESC(major, "major number for char device file");
module_param(option, int, S_IRUSR);
MODULE_PARM_DESC(option, "COM ports registration option");

#define BUFFER_SIZE 1024

struct uart16550_devdata
{
	struct cdev		cdev;
	long			addr;
	char			rdbuf[BUFFER_SIZE];
	int			rdget;
	int			rdput;
	atomic_t		rdfull;
	wait_queue_head_t	rdwq;
	char			wrbuf[BUFFER_SIZE];
	int			wrget;
	int			wrput;
	atomic_t		wrfull;
	wait_queue_head_t	wrwq;
};

struct uart16550_devdata devs[MAX_NUMBER_DEVICES];

static int uart16550_open(struct inode *inode, struct file *file)
{
	struct uart16550_devdata *data = container_of(inode->i_cdev,
			struct uart16550_devdata, cdev);
	file->private_data = data;
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

static void uart16550_set_line(struct uart16550_devdata *data, struct uart16550_line_info *uli)
{
	unsigned char lcr;
	unsigned short rate;

	lcr = inb(data->addr + REG_LCR_OFFSET);
	rate = uli->baud;

	/* Set Word Length, parity and stop bits */
	lcr = (lcr & 0xC0) | uli->len | uli->par | uli->stop;

	/* Disable interrupts */
	outb(0x00, data->addr + REG_IER_OFFSET);

	/* Set DLAB */
	outb(lcr | 0x80, data->addr + REG_LCR_OFFSET);

	outb(rate >> 8, data->addr + REG_DLH_OFFSET);
	outb(rate & 0xFF, data->addr + REG_DLL_OFFSET);

	/* Clear DLAB */
	outb(lcr & 0x7F, data->addr + REG_LCR_OFFSET);

	/* FIFO Control Register */
	//outb(0xC7, data->addr + REG_FCR_OFFSET);

	/* Turn on DTR, RTS and out2 */
	outb(0x0B, data->addr + REG_MCR_OFFSET);

	/* Enable interrupts */
	outb(0x03, data->addr + REG_IER_OFFSET);
}

static long uart16550_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct uart16550_devdata *data = (struct uart16550_devdata*) file->private_data;
	struct uart16550_line_info uli;
	if (cmd == UART16550_IOCTL_SET_LINE) {
		if (copy_from_user(&uli, (void*) arg,
			sizeof(struct uart16550_line_info))) {
			return -EFAULT;
		}
		uart16550_set_line(data, &uli);
	} else {
		return -EINVAL;
	}
	return 0;
}

irqreturn_t uart16550_interrupt_handle(int irq_no, void *dev_id)
{
	printk(KERN_DEBUG "[uart16550] in interrupt irq: %d\n", irq_no);
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

static dev_t first;
static dev_t com1;
static dev_t com2;
static int devcnt;

static int uart16550_init(void)
{
	int err;

	com1 = MKDEV(major, 0);
	com2 = MKDEV(major, 1);

	switch(option) {
	case OPTION_COM1:
		first = com1;
		devcnt = 1;
		break;
	case OPTION_COM2:
		first = com2;
		devcnt = 1;
		break;
	case OPTION_BOTH:
		first = com1;
		devcnt = 2;
		break;
	case 0:
		return 0;
	default:
		printk(KERN_ALERT "[uart16550] Invalid option: %d\n", option);
		return -EINVAL;
	}

	printk(KERN_DEBUG "[uart16550:%s] major=%d option=%d\n", __FUNCTION__, major, option);

	err = register_chrdev_region(first, devcnt, MODULE_NAME);
	if (err) {
		printk(KERN_ALERT "Unable to register char device region\n");
		goto err_register_chrdev;
	}

	if (option & UART16550_COM1_SELECTED) {
		devs[0].addr = COM1_BASE_ADDR;
		request_region(COM1_BASE_ADDR, NPORTS, MODULE_NAME);
		request_irq(IRQ_COM1, uart16550_interrupt_handle, IRQF_SHARED,
				MODULE_NAME, &devs[0]);
		cdev_init(&devs[0].cdev, &uart16550_fops);
		cdev_add(&devs[0].cdev, com1, 1);
	}

	if (option & UART16550_COM2_SELECTED) {
		devs[1].addr = COM2_BASE_ADDR;
		request_region(COM2_BASE_ADDR, NPORTS, MODULE_NAME);
		request_irq(IRQ_COM2, uart16550_interrupt_handle, IRQF_SHARED,
				MODULE_NAME, &devs[1]);
		cdev_init(&devs[1].cdev, &uart16550_fops);
		cdev_add(&devs[1].cdev, com2, 1);
	}

	return 0;

err_register_chrdev:
	return err;
}

static void uart16550_exit(void)
{
	int i;

	if (option & UART16550_COM1_SELECTED) {
		cdev_del(&devs[0].cdev);
		free_irq(IRQ_COM1, &devs[0]);
		release_region(COM1_BASE_ADDR, NPORTS);
	}
	if (option & UART16550_COM2_SELECTED) {
		cdev_del(&devs[1].cdev);
		free_irq(IRQ_COM2, &devs[1]);
		release_region(COM2_BASE_ADDR, NPORTS);
	}

	unregister_chrdev_region(first, devcnt);
}

module_init(uart16550_init);
module_exit(uart16550_exit);


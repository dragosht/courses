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
#include <linux/kfifo.h>

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

#define REG_THR_OFFSET 0 /* Transmit Holding Buffer Register */
#define REG_RBR_OFFSET 0 /* Receiver Buffer Register */
#define REG_DLL_OFFSET 0 /* Divisor Latch Low */
#define REG_IER_OFFSET 1 /* Interrupt Enable Register */
#define REG_DLH_OFFSET 1 /* Divisor Latch High */
#define REG_FCR_OFFSET 2 /* FIFO control Register */
#define REG_IIR_OFFSET 2 /* Interrupt Identification Register */
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
	struct kfifo		rdfifo;
	struct kfifo		wrfifo;
	spinlock_t		rdlock;
	spinlock_t		wrlock;
	wait_queue_head_t	rdwq;
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
	struct uart16550_devdata *dev = (struct uart16550_devdata*) file->private_data;
	int result = 0;

	if (wait_event_interruptible(dev->rdwq, kfifo_len(&dev->rdfifo) > 0)) {
		return -ERESTARTSYS;
	}

	count = min((size_t) kfifo_len(&dev->rdfifo), count);
	if (count <= 0) {
		return 0;
	}

	kfifo_to_user(&dev->rdfifo, buff,  count, &result);

	return result;
}

static ssize_t uart16550_write(struct file *file, const char __user *buff, size_t count, loff_t *offset)
{
	struct uart16550_devdata *dev = (struct uart16550_devdata*) file->private_data;
	int result = 0;

	if (wait_event_interruptible(dev->wrwq, kfifo_len(&dev->wrfifo) < BUFFER_SIZE)) {
		return -ERESTARTSYS;
	}

	count = min((size_t) (BUFFER_SIZE - kfifo_len(&dev->wrfifo)), count);
	if (count <= 0) {
		return 0;
	}

	kfifo_from_user(&dev->wrfifo, buff, count, &result);

	outb(0x00, dev->addr + REG_IER_OFFSET);
	outb(0x03, dev->addr + REG_IER_OFFSET);

	return result;
}

static void uart16550_set_line(struct uart16550_devdata *data, struct uart16550_line_info *uli)
{
	unsigned char lcr  = inb(data->addr + REG_LCR_OFFSET);
	lcr = (lcr & 0xC0) | uli->len | uli->par | uli->stop; //Word len, parity, stop
	outb(0x00, data->addr + REG_IER_OFFSET); //Disable interrupts
	outb(lcr | 0x80, data->addr + REG_LCR_OFFSET); //Set DLAB
	outb(uli->baud, data->addr + REG_DLL_OFFSET);
	outb(0x00, data->addr + REG_DLH_OFFSET);
	outb(lcr & 0x7F, data->addr + REG_LCR_OFFSET); //Clear DLAB
	//outb(0xC7, data->addr + REG_FCR_OFFSET); //Fifo Control Register
	outb(0x0B, data->addr + REG_MCR_OFFSET); //Turn on DTR, RTS, Auxout2
	outb(0x03, data->addr + REG_IER_OFFSET); //Enable interrupts
}

static long uart16550_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct uart16550_devdata *data = (struct uart16550_devdata*) file->private_data;
	struct uart16550_line_info uli;
	if (cmd == UART16550_IOCTL_SET_LINE) {
		if (copy_from_user(&uli, (void*) arg,
			sizeof(struct uart16550_line_info))) {
			printk(KERN_ALERT "[uart16550] Unable to set line parameters\n");
			return -EFAULT;
		}
		uart16550_set_line(data, &uli);
	} else {
		return -EINVAL;
	}
	return 0;
}

#define ETHR(lsr) ((lsr >> 5) & 0x01)
#define DREADY(lsr) (lsr & 0x01)
#define RDAI(iir) (((iir >> 1) & 0x03) == 0x02)
#define THREI(iir) (((iir >> 1) & 0x03) == 0x01)

irqreturn_t uart16550_interrupt_handle(int irq_no, void *dev_id)
{
	struct uart16550_devdata *dev = (struct uart16550_devdata*) dev_id;
	unsigned char iir;
	unsigned char val;

	if (dev != &devs[0] && dev != &devs[1]) {
		return IRQ_NONE;
	}

	iir = inb(dev->addr + REG_IIR_OFFSET);

	if (THREI(iir)) {
		if (kfifo_len(&dev->wrfifo) > 0) {
			kfifo_out(&dev->wrfifo, &val, 1);
			outb(val, dev->addr + REG_THR_OFFSET);
			wake_up_interruptible(&dev->wrwq);
		}
	} else if (RDAI(iir)) {
		if (kfifo_len(&dev->rdfifo) < BUFFER_SIZE) {
			val = inb(dev->addr + REG_RBR_OFFSET);
			kfifo_in(&dev->rdfifo, &val, 1);
			wake_up_interruptible(&dev->rdwq);
		}
	}

	return IRQ_HANDLED;
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
static int devcnt;

struct uart_port
{
	dev_t	dev;
	int	irq;
	int	addr;
	int	opt;
};

static struct uart_port ports[MAX_NUMBER_DEVICES];

static int uart16550_init(void)
{
	int i, err;

	ports[0].dev = MKDEV(major, 0);
	ports[0].irq = IRQ_COM1;
	ports[0].addr = COM1_BASE_ADDR;
	ports[0].opt = UART16550_COM1_SELECTED;

	ports[1].dev = MKDEV(major, 1);
	ports[1].irq = IRQ_COM2;
	ports[1].addr = COM2_BASE_ADDR;
	ports[1].opt = UART16550_COM2_SELECTED;

	switch(option) {
	case OPTION_COM1:
		first = ports[0].dev;
		devcnt = 1;
		break;
	case OPTION_COM2:
		first = ports[1].dev;
		devcnt = 1;
		break;
	case OPTION_BOTH:
		first = ports[0].dev;
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

	for (i = 0; i < MAX_NUMBER_DEVICES; i++) {
		if (!(option & ports[i].opt)) {
			continue;
		}
		devs[i].addr = ports[i].addr;
		kfifo_alloc(&devs[i].rdfifo, BUFFER_SIZE, 0);
		kfifo_alloc(&devs[i].wrfifo, BUFFER_SIZE, 0);
		spin_lock_init(&devs[i].rdlock);
		spin_lock_init(&devs[i].wrlock);
		init_waitqueue_head(&devs[i].rdwq);
		init_waitqueue_head(&devs[i].wrwq);
		/* TBD: What to do if only one device registration fails? */
		if (request_region(ports[i].addr, NPORTS, MODULE_NAME) == NULL) {
			printk("[uart16550] Unable to register port region: %x\n",
					ports[i].addr);
			goto err_region;
		}
		if (request_irq(ports[i].irq, uart16550_interrupt_handle,
				IRQF_SHARED, MODULE_NAME, &devs[i])) {
			printk("[uart16550] Unable to get irq: %d\n", ports[i].irq);
			goto err_irq;
		}
		cdev_init(&devs[i].cdev, &uart16550_fops);
		cdev_add(&devs[i].cdev, ports[i].dev, 1);
		continue;

	err_irq:
		unregister_chrdev_region(ports[i].addr, NPORTS);
	err_region:
		continue;
	}

	return 0;

err_register_chrdev:
	return err;
}

static void uart16550_exit(void)
{
	int i;
	for (i = 0; i < MAX_NUMBER_DEVICES; i++) {
		if (!(option & ports[i].opt)) {
			continue;
		}
		cdev_del(&devs[i].cdev);
		free_irq(ports[i].irq, &devs[i]);
		release_region(ports[i].addr, NPORTS);
		kfifo_free(&devs[i].rdfifo);
		kfifo_free(&devs[i].wrfifo);
	}
	unregister_chrdev_region(first, devcnt);
}

module_init(uart16550_init);
module_exit(uart16550_exit);


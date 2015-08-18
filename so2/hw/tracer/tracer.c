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
#include <asm/uaccess.h>

#include "tracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

struct task_kmalloc_entry
{
	unsigned long		address;
	unsigned long		size;
	struct hlist_node	hnode;
};

struct task_stats {
	pid_t			pid;
	unsigned long		kmalloc_count;
	unsigned long		kfree_count;
	unsigned long		kmalloc_mem;
	unsigned long		kfree_mem;
	unsigned long		sched_count;
	unsigned long		up_count;
	unsigned long		down_count;
	unsigned long		lock_count;
	unsigned long		unlock_count;
	struct hlist_node	hnode;
	struct hlist_head	hkmalloced;
};

struct hlist_head	hpids;

enum jprobe_type {
	JPROBE_KFREE = 0,
	JPROBE_SCHEDULE,
	JPROBE_UP,
	JPROBE_DOWN,
	JPROBE_LOCK,
	JPROBE_UNLOCK,
	JPROBE_EXIT,
	JPROBE_COUNT
};

void jprobe_kfree(const void *objp)
{
	//printk(KERN_DEBUG "jprobe_kfree\n");
	jprobe_return();
}

asmlinkage void __sched jprobe_schedule(void)
{
	//printk(KERN_DEBUG "jprobe_schedule\n");
	//
	jprobe_return();
}

void jprobe_up(struct semaphore *sem)
{
	//printk(KERN_DEBUG "jprobe_up\n");
	jprobe_return();
}

int jprobe_down_interruptible(struct semaphore *sem)
{
	//printk(KERN_DEBUG "jprobe_down_interruptible\n");
	jprobe_return();
	return 0;
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC
void __sched jprobe_lock(struct mutex *lock, unsigned int subclass)
#else
void __sched jprobe_lock(struct mutex *lock)
#endif
{
	//printk(KERN_DEBUG "jprobe_lock\n");
	jprobe_return();
}

void __sched jprobe_unlock(struct mutex *lock)
{
	//printk(KERN_DEBUG "jprobe_unlock\n");
	jprobe_return();
}

void jprobe_exit(long code)
{
    printk(KERN_DEBUG "jprobe_exit: pid %d exited\n", current->pid);
    jprobe_return();
}


void* jprobe_handlers[] = {
	jprobe_kfree,
	jprobe_schedule,
	jprobe_up,
	jprobe_down_interruptible,
	jprobe_lock,
	jprobe_unlock,
	jprobe_exit
};

const char* jprobe_symnames[] = {
	"kfree",
	"schedule",
	"up",
	"down_interruptible",
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	"mutex_lock_nested",
#else
	"mutex_lock",
#endif
	"mutex_unlock",
	"do_exit"
};

static int kmalloc_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	//printk(KERN_ALERT "in kmalloc\n");
	return 0;
}

struct kretprobe    probe_kmalloc;
struct jprobe       jprobes[JPROBE_COUNT];

static int tracer_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int tracer_release(struct inode *inode, struct file *file)
{
	return 0;
}

static loff_t tracer_lseek(struct file *file, loff_t offset, int whence)
{
	return 0;
}

static ssize_t tracer_read(struct file *file, char *buff, size_t n, loff_t *offset)
{
	return -EPERM;
}

static ssize_t tracer_write(struct file *file, const char *buff, size_t n, loff_t *offset)
{
	return -EPERM;
}

long tracer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

struct file_operations tracer_fops = {
	owner:			THIS_MODULE,
	open:			tracer_open,
	release:		tracer_release,
	read:			tracer_read,
	write:			tracer_write,
	llseek:			tracer_lseek,
	unlocked_ioctl:		tracer_ioctl,
};

struct miscdevice	device;

static int tracer_init(void)
{
	int i;
	int status;

	device.minor = TRACER_DEV_MINOR;
	device.name = "tracer";
	device.fops = &tracer_fops;
	status = misc_register(&device);
	if (status) {
		printk(KERN_ALERT "Unable to register misc device\n");
		return status;
	}

	probe_kmalloc.handler = kmalloc_handler;
	probe_kmalloc.kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("__kmalloc");
	if (!probe_kmalloc.kp.addr) {
		printk(KERN_ALERT "Cannot find %s to plant kretprobe\n", "__kmalloc");
		status = -EINVAL;
		goto err_kmalloc_register;
	}
	register_kretprobe(&probe_kmalloc);

	for (i = 0; i < JPROBE_COUNT; i++) {
		jprobes[i].entry = (kprobe_opcode_t*) jprobe_handlers[i];
		jprobes[i].kp.addr = (kprobe_opcode_t*)
			kallsyms_lookup_name(jprobe_symnames[i]);
		if (!jprobes[i].kp.addr) {
			printk(KERN_ALERT "Could not find %s to plant jprobe\n",
			       jprobe_symnames[i]);
			status = -EINVAL;
			goto err_jprobe_register;
		}
		status = register_jprobe(&jprobes[i]);
		if (status) {
			printk(KERN_ALERT "Unable to register jprobe for: %s\n",
				jprobe_symnames[i]);
			goto err_jprobe_register;
		}
	}

	hash_init(hpids);

	return 0;

err_jprobe_register:
	i--;
	while (i >= 0) {
		unregister_jprobe(&jprobes[i]);
	}
	unregister_kretprobe(&probe_kmalloc);
err_kmalloc_register:
	misc_deregister(&device);
	return status;
}

static void tracer_exit(void)
{
	int i;
	for (i = 0; i < JPROBE_COUNT; i++) {
		unregister_jprobe(&jprobes[i]);
	}
	unregister_kretprobe(&probe_kmalloc);
	misc_deregister(&device);
}

module_init(tracer_init);
module_exit(tracer_exit);


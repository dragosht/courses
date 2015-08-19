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

#include "tracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define HKMALLOC_SIZE (1 << 8)

#define procfs_tracer_read	"tracer"
static struct proc_dir_entry *proc_tracer_read;

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
	struct hlist_head	hkmalloced[HKMALLOC_SIZE];
};

void task_stats_remove(struct task_stats* pts)
{
	int bkt;
	struct hlist_node *tmp;
	struct task_kmalloc_entry *ptme;

	if (!pts) {
		return;
	}
	/* clear the hkmalloced hash */
	hash_for_each_safe(pts->hkmalloced, bkt, tmp, ptme, hnode) {
		hash_del(&ptme->hnode);
		kfree(ptme);
	}
	kfree(pts);
}

DEFINE_HASHTABLE(hpids, 16);

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

/* void * kmalloc(size_t size, int flags) */
static int kmalloc_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pid_t pid;
	struct task_stats *pts;
	struct task_kmalloc_entry* ptke;


	ptke = (struct task_kmalloc_entry*) ri->data;
	pid = current->pid;

	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			ptke->size = regs->ax;
		}
	}

	/* No point to continue tracing this one ... */
	return 1;
}

static int kmalloc_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pid_t pid;
	struct task_stats *pts;
	struct task_kmalloc_entry* ptke;
	struct task_kmalloc_entry* ntke;

	ptke = (struct task_kmalloc_entry*) ri->data;
	pid = current->pid;

	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			ptke->address = (unsigned long) ri->ret_addr;
			ntke = kmalloc(sizeof(struct task_kmalloc_entry), GFP_KERNEL);
			if (!ntke) {
				return -ENOMEM;
			}
			ntke->address = ptke->address;
			ntke->size = ptke->size;
			hash_add(pts->hkmalloced, &ntke->hnode, ntke->address);

			pts->kmalloc_count++;
			pts->kmalloc_mem += ntke->size;
		}
	}

	return 0;
}

void jprobe_kfree(const void *objp)
{
	pid_t pid;
	struct task_stats *pts;
	struct task_kmalloc_entry *ptke;
	struct hlist_node *tmp;
	unsigned long address;
	unsigned long size;
	int found;

	pid = current->pid;
	address = (unsigned long) objp;

	found = 0;
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			found = 1;
			break;
		}
	}
	if (!found) {
		goto not_found;
	}

	size = 0;
	hash_for_each_possible_safe(pts->hkmalloced, ptke, tmp, hnode, address) {
		if (ptke->address == address) {
			size = ptke->size;
			hash_del(&ptke->hnode);
		}
	}

	pts->kfree_count++;
	pts->kfree_mem += size;

not_found:
	jprobe_return();
}

asmlinkage void __sched jprobe_schedule(void)
{
	pid_t pid;
	struct task_stats *pts;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_schedule\n");
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			pts->sched_count++;
		}
	}

	jprobe_return();
}

void jprobe_up(struct semaphore *sem)
{
	pid_t pid;
	struct task_stats *pts;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_up\n");
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			pts->up_count++;
		}
	}

	jprobe_return();
}

int jprobe_down_interruptible(struct semaphore *sem)
{
	pid_t pid;
	struct task_stats *pts;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_down_interruptible\n");
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			pts->down_count++;
		}
	}

	jprobe_return();
	return 0;
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC
void __sched jprobe_lock(struct mutex *lock, unsigned int subclass)
#else
void __sched jprobe_lock(struct mutex *lock)
#endif
{
	pid_t pid;
	struct task_stats *pts;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_lock\n");
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			pts->lock_count++;
		}
	}

	jprobe_return();
}

void __sched jprobe_unlock(struct mutex *lock)
{
	pid_t pid;
	struct task_stats *pts;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_unlock\n");
	hash_for_each_possible(hpids, pts, hnode, pid) {
		if (pts->pid == pid) {
			pts->unlock_count++;
		}
	}

	jprobe_return();
}

void jprobe_exit(long code)
{
	pid_t pid;
	struct task_stats *pts;
	struct hlist_node *tmp;

	pid = current->pid;

	//printk(KERN_DEBUG "jprobe_exit: pid %d exited\n", current->pid);
	hash_for_each_possible_safe(hpids, pts, tmp, hnode, pid) {
		if (pts->pid == pid) {
			hash_del(&pts->hnode);
			task_stats_remove(pts);
		}
	}

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
	return -EPERM;
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
	pid_t pid = (pid_t) arg;
	struct task_stats *pts;
	struct hlist_node *tmp;

	switch(cmd) {
	case TRACER_ADD_PROCESS:
		/* First check if the process is already in ... */
		hash_for_each_possible(hpids, pts, hnode, pid) {
			if (pts->pid == pid) {
				return -EINVAL;
			}
		}
		pts = kmalloc(sizeof(struct task_stats), GFP_KERNEL);
		if (!pts) {
			return -ENOMEM;
		}

		memset(pts, 0, sizeof(struct task_stats));
		pts->pid = pid;
		hash_add(hpids, &pts->hnode, pts->pid);
		break;

	case TRACER_REMOVE_PROCESS:
		/* Look for the given pid and wipe it */
		hash_for_each_possible_safe(hpids, pts, tmp, hnode, pid) {
			if (pts->pid == pid) {
				hash_del(&pts->hnode);
				task_stats_remove(pts);
			}
		}
		break;
	default:
		return -EINVAL;
	}
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


static int tracer_proc_show(struct seq_file *m, void *v)
{
	struct task_stats *pts;
	int bkt;

	seq_printf(m, "%-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s\n",
		"PID",
		"kmalloc", "kfree",
		"kmalloc_mem", "kfree_mem",
		"sched",
		"up", "down",
		"lock", "unlock");

	hash_for_each(hpids, bkt, pts, hnode) {
		seq_printf(m, "%7d %7lu %7lu %7lu %7lu %7lu %7lu %7lu %7lu %7lu\n",
			pts->pid,
			pts->kmalloc_count, pts->kfree_count,
			pts->kmalloc_mem, pts->kfree_mem,
			pts->sched_count,
			pts->up_count, pts->down_count,
			pts->lock_count, pts->unlock_count);
	}

	return 0;
}

static int tracer_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, tracer_proc_show, NULL);
}

struct file_operations tracer_proc_fops = {
	owner:			THIS_MODULE,
	open:			tracer_proc_open,
	read:			seq_read,
	release:		single_release
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

	proc_tracer_read = proc_create(procfs_tracer_read, 0, 0, &tracer_proc_fops);
	if (!proc_tracer_read) {
		printk(KERN_ALERT "Unable to create /proc/tracer\n");
		goto err_proc_create;
	}

	probe_kmalloc.entry_handler = kmalloc_entry_handler;
	probe_kmalloc.handler = kmalloc_handler;
	probe_kmalloc.data_size = sizeof(struct task_kmalloc_entry);
	probe_kmalloc.maxactive = 10;
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

	return 0;

err_jprobe_register:
	i--;
	while (i >= 0) {
		unregister_jprobe(&jprobes[i]);
	}
	unregister_kretprobe(&probe_kmalloc);
err_kmalloc_register:
	proc_remove(proc_tracer_read);
err_proc_create:
	misc_deregister(&device);
	return status;
}

static void tracer_exit(void)
{
	int i;
	int bkt;
	struct hlist_node *tmp;
	struct task_stats *pts;

	for (i = 0; i < JPROBE_COUNT; i++) {
		unregister_jprobe(&jprobes[i]);
	}
	unregister_kretprobe(&probe_kmalloc);

	hash_for_each_safe(hpids, bkt, tmp, pts, hnode) {
		hash_del(&pts->hnode);
		task_stats_remove(pts);
	}

	proc_remove(proc_tracer_read);
	misc_deregister(&device);
}

module_init(tracer_init);
module_exit(tracer_exit);


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

#include "tracer.h"

MODULE_DESCRIPTION("Kprobe based tracer");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

struct list_head head;

struct task_stats {
	pid_t       pid;
	int         count_kmalloc;
	int         count_kfree;
	int         count_kmalloc_mem;
	int         count_kfree_mem;
	int         count_sched;
	int         count_up;
	int         count_down;
	int         count_lock;
	int         count_unlock;
	struct list_head list;
};

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

asmlinkage long jprobe_exit(int error_code)
{
	printk(KERN_DEBUG "jprobe_exit: pid %d exited\n", current->pid);
	jprobe_return();
	return 0;
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
	"sys_exit"
};

static int kmalloc_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	//printk(KERN_ALERT "in kmalloc\n");
	return 0;
}

struct kretprobe    probe_kmalloc;
struct jprobe       jprobes[JPROBE_COUNT];

static int tracer_init(void)
{
	int i;
	int status;

	probe_kmalloc.handler = kmalloc_handler;
	probe_kmalloc.kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("__kmalloc");
	if (!probe_kmalloc.kp.addr) {
		printk(KERN_ALERT "Cannot find %s to plant kretprobe\n", "__kmalloc");
		return -1;
	}
	register_kretprobe(&probe_kmalloc);

	for (i = 0; i < JPROBE_COUNT; i++) {
		jprobes[i].entry = (kprobe_opcode_t*) jprobe_handlers[i];
		jprobes[i].kp.addr = (kprobe_opcode_t*)
			kallsyms_lookup_name(jprobe_symnames[i]);
		if (!jprobes[i].kp.addr) {
			printk(KERN_ALERT "Could not find %s to plant jprobe\n",
			       jprobe_symnames[i]);
			return -1;
		}
	}

	for (i = 0; i < JPROBE_COUNT; i++) {
		status = register_jprobe(&jprobes[i]);
		if (status < 0) {
			printk(KERN_ALERT "Unable to register jprobe: %s\n",
					jprobe_symnames[i]);
			return status;
		} else {
			printk(KERN_ALERT "Successfully registered jprobe: %s\n",
					jprobe_symnames[i]);
		}
	}

	return 0;
}

static void tracer_exit(void)
{
	int i;
	for (i = 0; i < JPROBE_COUNT; i++) {
		unregister_jprobe(&jprobes[i]);
	}
	kfree(jprobes);
	unregister_kretprobe(&probe_kmalloc);
}

module_init(tracer_init);
module_exit(tracer_exit);



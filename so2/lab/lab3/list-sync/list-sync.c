/*
 * SO2 lab3 - task 5
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("Full list processing");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_ALERT

struct task_info {
	pid_t pid;
	unsigned long timestamp;
	atomic_t count;
	struct list_head list;
};

static struct list_head head;
static spinlock_t lock;

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

        ti = kmalloc(sizeof(struct task_info), GFP_KERNEL);
        if (ti == NULL) {
                return NULL;
        }

        ti->pid = pid;
        ti->timestamp = jiffies;

	atomic_set(&ti->count, 0);

	return ti;
}

/*
 * Return pointer to struct task_info structure or NULL in case
 * pid argument isn't in the list.
 */

static struct task_info *task_info_find_pid(int pid)
{
	struct list_head *p;
	struct task_info *ti;

        list_for_each(p, &head) {
                ti = list_entry(p, struct task_info, list);
                if (ti->pid == pid) {
                        return ti;
                }
        }
        return NULL;
}

static void task_info_add_to_list(int pid)
{
	struct task_info *ti;

        spin_lock(&lock);
	ti = task_info_find_pid(pid);
	if (ti != NULL) {
		ti->timestamp = jiffies;
		atomic_inc(&ti->count);
                spin_unlock(&lock);
		return;
	}

        ti = task_info_alloc(pid);
        if (ti) {
                list_add(&ti->list, &head);
        }
        spin_unlock(&lock);
}

void task_info_add_for_current(void)
{
        task_info_add_to_list(current->pid);
	task_info_add_to_list(current->real_parent->pid);
	task_info_add_to_list(next_task(current)->pid);
	task_info_add_to_list(next_task(next_task(current))->pid);
}

EXPORT_SYMBOL(task_info_add_for_current);

void task_info_print_list(const char *msg)
{
	struct list_head *p;
	struct task_info *ti;

        spin_lock(&lock);
	printk(LOG_LEVEL "%s: [ ", msg);
	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		printk("(%d, %lu) ", ti->pid, ti->timestamp);
	}
	printk("]\n");
        spin_unlock(&lock);
}

EXPORT_SYMBOL(task_info_print_list);

void task_info_remove_expired(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

        spin_lock(&lock);
	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		if (jiffies - ti->timestamp > 3 * HZ && atomic_read(&ti->count) < 5) {
			list_del(p);
			kfree(ti);
		}
	}
        spin_unlock(&lock);
}

EXPORT_SYMBOL(task_info_remove_expired);

static void task_info_purge_list(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

        spin_lock(&lock);
	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		list_del(p);
		kfree(ti);
	}
        spin_unlock(&lock);
}

static int list_full_init(void)
{
	INIT_LIST_HEAD(&head);

	task_info_add_for_current();
	task_info_print_list("after first add");

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);

	return 0;
}

static void list_full_exit(void)
{
        /*
        struct task_info *ti = list_first_entry(&head, struct task_info, list);
        if (ti) {
                atomic_set(&ti->count, 20);
        }
        */

        task_info_print_list("before expiring");

	task_info_remove_expired();
	task_info_print_list("after removing expired");
	task_info_purge_list();
}

module_init(list_full_init);
module_exit(list_full_exit);


/*
 * simple hash list example
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("Simple radix test");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

struct fops_by_pid {
	unsigned long   somedata;
	pid_t		pid;
};

pid_t pids[] = {600, 605, 567, 568, 569, 570, 571, 572, 573};

struct radix_tree_root rtree;

static int radixtest_init(void)
{
	void **slot;
	struct radix_tree_iter iter;
	int i;

	INIT_RADIX_TREE(&rtree, GFP_KERNEL);

	for (i = 0; i < sizeof(pids) / sizeof(pid_t); i++) {
		struct fops_by_pid *fbp = kmalloc(sizeof(struct fops_by_pid), GFP_KERNEL);
		if (!fbp) {
			printk(KERN_ALERT "No more space!\n");
			return -ENOMEM;
		}
		fbp->somedata = 0;
		fbp->pid = pids[i];
		radix_tree_insert(&rtree, fbp->pid, fbp);
	}

	radix_tree_for_each_slot(slot, &rtree, &iter, 0) {
		struct fops_by_pid *fbp = radix_tree_deref_slot(slot);
		printk("slot: %p PID: %d\n", slot, fbp->pid);
	}

	return 0;
}

static void radixtest_exit(void)
{
	void **slot;
	struct radix_tree_iter iter;

	radix_tree_for_each_slot(slot, &rtree, &iter, 0) {
		struct fops_by_pid *fbp = radix_tree_deref_slot(slot);
		radix_tree_delete(&rtree, fbp->pid);
		kfree(fbp);
	}
}

module_init(radixtest_init);
module_exit(radixtest_exit);


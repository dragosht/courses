/*
 * simple hash list example
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("Simple hash test");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

/* Let's try indexing something by pids ... */
struct fops_by_pid {
	unsigned long	open_count;
	unsigned long	release_count;
	unsigned long	read_count;
	unsigned long	write_count;
	unsigned long	lseek_count;
	unsigned long	ioctl_count;
	pid_t		pid;
	struct hlist_node node;
};

pid_t pids[] = {600, 605, 567, 568, 569, 570, 571, 572, 573};

/* This expands to ... */
/*
DEFINE_HASHTABLE(hhead, 12);
*/
struct hlist_head hhead[4096];


/* Looks like this API suffered some changes after 3.13 ... */

static int hashtest_init(void)
{
	int i;
	int bucket;
	struct fops_by_pid *fbp;

	for (i = 0; i < sizeof(pids) / sizeof(pid_t); i++) {
		struct fops_by_pid *fbp = kmalloc(sizeof(struct fops_by_pid), GFP_KERNEL);
		if (!fbp) {
			printk(KERN_ALERT "No more space!\n");
			return -ENOMEM;
		}
		fbp->pid = pids[i];
		fbp->open_count = 0;
		fbp->release_count = 0;
		fbp->read_count = 0;
		fbp->write_count = 0;
		fbp->lseek_count = 0;
		fbp->ioctl_count = 0;
		hash_add(hhead, &fbp->node, fbp->pid);
	}

	hash_for_each(hhead, bucket, fbp, node) {
		printk("entry: %d %lu %lu %lu %lu %lu %lu\n",
			fbp->pid,
			fbp->open_count,
			fbp->release_count,
			fbp->read_count,
			fbp->write_count,
			fbp->lseek_count,
			fbp->ioctl_count);
	}

	return 0;
}

static void hashtest_exit(void)
{
	int bucket;
	struct hlist_node* tmp;
	struct fops_by_pid *fbp;

	hash_for_each_safe(hhead, bucket, tmp, fbp, node) {
		hash_del(&fbp->node);
		kfree(fbp);
	}
}

module_init(hashtest_init);
module_exit(hashtest_exit);


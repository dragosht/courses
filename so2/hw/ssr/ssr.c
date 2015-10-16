#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>

#include "ssr.h"

MODULE_AUTHOR("Dragos Tarcatu");
MODULE_DESCRIPTION("Simple Software RAID");
MODULE_LICENSE("GPL");

struct ssr_device
{
	struct request_queue *queue;
	struct gendisk *gd;
	spinlock_t lock;
};

static struct ssr_device ssr_dev;

static void ssr_request(struct request_queue *queue, struct bio *bio)
{
}

static int ssr_create_device(struct ssr_device* dev)
{
	int err = 0;
	dev->queue = blk_alloc_queue(GFP_KERNEL);
	if (dev->queue == NULL) {
		printk (KERN_ERR "Unable to allocate block device queue\n");
		return -ENOMEM;
	}
	blk_queue_make_request(dev->queue, ssr_request);
	dev->queue->queuedata = dev;

}

static int __init ssr_init(void)
{
	int err = 0;
	err = register_blkdev(SSR_MAJOR, LOGICAL_DISK_NAME);
	if (err) {
		printk(KERN_ERR "Unable to register ssr block device\n");
		return -EBUSY;
	}
	err = ssr_create_device(&ssr_dev);
	if (err) {
		goto out;
	}
	return 0;
out:
	unregister_blkdev(SSR_MAJOR, LOGICAL_DISK_NAME);
	return err;
}

static void __exit ssr_exit(void)
{
	unregister_blkdev(SSR_MAJOR, LOGICAL_DISK_NAME);
}

module_init(ssr_init);
module_exit(ssr_exit);


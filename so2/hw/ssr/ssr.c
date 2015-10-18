#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/workqueue.h>
#include <linux/crc32.h>

#include "ssr.h"

MODULE_AUTHOR("Dragos Tarcatu");
MODULE_DESCRIPTION("Simple Software RAID");
MODULE_LICENSE("GPL");

struct ssr_device
{
	struct block_device *phys_bdev1;
	struct block_device *phys_bdev2;
	struct request_queue *queue;
	struct workqueue_struct *wq;
	struct work_struct *ws;
	struct bio *bio;
	struct gendisk *gd;
	spinlock_t lock;
};

static struct ssr_device ssr_dev;

static int ssr_open(struct block_device *bdev, fmode_t mode)
{
    return 0;
}

static void ssr_release(struct gendisk *gd, fmode_t mode)
{
}

static const struct block_device_operations ssr_ops = {
    .owner = THIS_MODULE,
    .open = ssr_open,
    .release = ssr_release
};



/*
 * CRCs are between:
 * start = LOGICAL_DISK_SECTORS
 * stop = start + sizeof(u32) * LOGICAL_DISK_SECTORS
 *
 * There are (KERNEL_SECTOR_SIZE / sizeof(u32)) CRCs in each sector (= 128)
 *
 * for sector s the CRC is in sector: start + s / (KERNEL_SECTOR_SIZE / sizeof(u32))
 *
 */

static void crc_complete(struct bio *bio, int error)
{
	/* This one runs in interrupt context */
	printk(KERN_DEBUG "CRC read completed\n");
	complete((struct completion*)bio->bi_private);
}

static void bi_complete(struct bio *bio, int error)
{
	/* This one runs in interrupt context */
	printk(KERN_DEBUG "Read completed\n");
	complete((struct completion*)bio->bi_private);
}

#define CRCS_PER_SECTOR (KERNEL_SECTOR_SIZE / sizeof(u32))
#define CRCSTART (LOGICAL_DISK_SECTORS)
#define CRCSECT(sect) (CRCSTART + ((sect) / CRCS_PER_SECTOR))

static void read_crc(sector_t first_sector, int num_sectors,
	sector_t first_crc_sector, int num_crc_sectors,
	struct block_device *bdev, u32 *crc)
{
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	struct completion event;
	struct page *page;
	char *buffer;
	u32 *crcs;
	int ndx;
	int i;

	bio->bi_bdev = bdev;
	bio->bi_sector = first_crc_sector;
	bio->bi_rw = 0;
	init_completion(&event);
	bio->bi_private = &event;
	bio->bi_end_io = crc_complete;

	page = alloc_page(GFP_NOIO);
	bio_add_page(bio, page, num_crc_sectors * KERNEL_SECTOR_SIZE, 0);
	bio->bi_vcnt = 1;
	bio->bi_idx = 0;

	submit_bio(0, bio);
	wait_for_completion(&event);

	buffer = __bio_kmap_atomic(bio, 0);
	crcs = (u32*)buffer;
	/* Now where exactly are the CRCs I need?... */
	ndx = first_sector % CRCS_PER_SECTOR;
	for (i = 0; i < num_sectors; i++) {
		crc[i] = crcs[ndx + i];
	}
	__bio_kunmap_atomic(buffer);

	bio_put(bio);
	__free_page(page);
}

void ssr_work_handler(struct work_struct *work)
{
	struct bio *bio = ssr_dev.bio;
	struct completion event;
	sector_t first_sector;
	sector_t first_crc_sector;
	sector_t last_crc_sector;
	int num_crc_sectors;
	int num_sectors;
	u32 *crcd1;
	u32 *crcd2;

	if (!bio) {
		return;
	}

	/* first read the CRCs */
	num_sectors  = bio_sectors(bio);
	first_sector = bio->bi_sector;

	first_crc_sector = CRCSECT(first_sector);
	last_crc_sector = CRCSECT(first_sector + num_sectors);
	num_crc_sectors = last_crc_sector - first_crc_sector + 1;

	printk(KERN_DEBUG "CRCS: first_sector: %llu last_sector: %llu num_sectors: %d\n",
			first_crc_sector, last_crc_sector, num_crc_sectors);

	crcd1 = kmalloc(num_sectors * sizeof(u32), GFP_KERNEL);
	crcd2 = kmalloc(num_sectors * sizeof(u32), GFP_KERNEL);

	if (!crcd1 || !crcd2) {
		printk(KERN_ERR "Unable to allocate space for CRCs\n");
		return;
	}

	/* First read the crcs for the sectors affected by this operation */
	read_crc(first_sector, num_sectors, first_crc_sector,
		num_crc_sectors, ssr_dev.phys_bdev1, crcd1);
	read_crc(first_sector, num_sectors, first_crc_sector,
		num_crc_sectors, ssr_dev.phys_bdev2, crcd2);

	if (bio_data_dir(bio)) {
		/* Propagate writes and adjust CRCs */
	} else {
		/* Propagate reads and check errors */
		struct bio *bio1 = bio_clone(bio, GFP_NOIO);
		struct bio *bio2 = bio_clone(bio, GFP_NOIO);
		struct completion event1;
		struct completion event2;
		char *buffer;
		int i;

		bio1->bi_bdev = ssr_dev.phys_bdev1;
		init_completion(&event1);
		bio1->bi_private = &event1;
		bio1->bi_end_io = bi_complete;
		submit_bio(0, bio1);
		wait_for_completion(&event1);

		buffer = __bio_kmap_atomic(bio1, 0);
		for (i = 0; i < num_sectors; i++) {
			u32 crc = crc32(0, buffer + i * KERNEL_SECTOR_SIZE,
				KERNEL_SECTOR_SIZE);
			if (crc != crcd1[i]) {
				printk(KERN_ERR "invalid disk1 CRC value\n");
			}
		}
		__bio_kunmap_atomic(buffer);

		bio2->bi_bdev = ssr_dev.phys_bdev2;
		init_completion(&event2);
		bio2->bi_private = &event2;
		bio2->bi_end_io = bi_complete;
		submit_bio(0, bio2);
		wait_for_completion(&event2);

		buffer = __bio_kmap_atomic(bio2, 0);
		for (i = 0; i < num_sectors; i++) {
			u32 crc = crc32(0, buffer + i * KERNEL_SECTOR_SIZE,
				KERNEL_SECTOR_SIZE);
			if (crc != crcd2[i]) {
				printk(KERN_ERR "invalid disk2 CRC value\n");
			}
		}
		__bio_kunmap_atomic(buffer);

	}

	kfree(crcd1);
	kfree(crcd2);
}

static void ssr_do_bio(struct bio *bio)
{
	struct work_struct work;
	INIT_WORK(&work, ssr_work_handler);
	ssr_dev.bio = bio;
	schedule_work(&work);
	flush_scheduled_work();
	//queue_work(ssr_dev.wq, &work);
	//flush_workqueue(ssr_dev.wq);
}

static void ssr_make_request(struct request_queue *queue, struct bio *bio)
{
	struct bio_vec *bvec;
	int i;

	printk(KERN_DEBUG "bio: %8p dir: %d sector: %llu sectors: %d\n",
		bio, bio_data_dir(bio), bio->bi_sector, bio_sectors(bio));

	ssr_do_bio(bio);

	bio_endio(bio, 0);
}

static int open_disks(struct ssr_device *dev)
{
	int err = 0;

	dev->phys_bdev1 = blkdev_get_by_path(PHYSICAL_DISK1_NAME,
		FMODE_READ | FMODE_WRITE | FMODE_EXCL, THIS_MODULE);
	if (dev->phys_bdev1 == NULL) {
		printk(KERN_ERR PHYSICAL_DISK1_NAME "No such device\n");
		return -EINVAL;
	}

	dev->phys_bdev2 = blkdev_get_by_path(PHYSICAL_DISK2_NAME,
		FMODE_READ | FMODE_WRITE | FMODE_EXCL, THIS_MODULE);
	if (dev->phys_bdev2 == NULL) {
		printk(KERN_ERR PHYSICAL_DISK2_NAME "No such device\n");
		err = -EINVAL;
		goto err_disk;
	}

	return err;
err_disk:
	blkdev_put(dev->phys_bdev1, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	return err;

}

static void close_disks(struct ssr_device *dev)
{
	blkdev_put(dev->phys_bdev1, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	blkdev_put(dev->phys_bdev2, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
}

static int ssr_create_device(struct ssr_device* dev)
{
	int err = 0;
        memset(dev, 0, sizeof(struct ssr_device));

	dev->wq = create_workqueue("ssr_workqueue");

	err = open_disks(dev);
	if (err) {
		goto err_open_disks;
	}

	dev->queue = blk_alloc_queue(GFP_KERNEL);
	if (dev->queue == NULL) {
		printk (KERN_ERR "Unable to allocate block device queue\n");
		err = -ENOMEM;
                goto err_alloc_queue;
	}
	blk_queue_make_request(dev->queue, ssr_make_request);
        blk_queue_logical_block_size(dev->queue, KERNEL_SECTOR_SIZE);
	dev->queue->queuedata = dev;

        dev->gd = alloc_disk(SSR_NUM_MINORS);
        if (!dev->gd) {
            printk(KERN_ERR "Unable to allocate disk\n");
            err = -ENOMEM;
            goto err_alloc_disk;
        }

        dev->gd->major = SSR_MAJOR;
        dev->gd->first_minor = SSR_FIRST_MINOR;
        dev->gd->fops = &ssr_ops;
        dev->gd->queue = dev->queue;
        dev->gd->private_data = dev;
        snprintf(dev->gd->disk_name, strlen(LOGICAL_DISK_NAME), LOGICAL_DISK_NAME);
        set_capacity(dev->gd, LOGICAL_DISK_SECTORS);

        add_disk(dev->gd);

        return err;

err_alloc_disk:
	blk_cleanup_queue(dev->queue);

err_alloc_queue:
	close_disks(dev);

err_open_disks:
        return err;
}

static void ssr_delete_device(struct ssr_device *dev)
{
	flush_workqueue(dev->wq);
	destroy_workqueue(dev->wq);

	if (dev->gd) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}
	if (dev->queue) {
		blk_cleanup_queue(dev->queue);
	}
	close_disks(dev);
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
	ssr_delete_device(&ssr_dev);
	unregister_blkdev(SSR_MAJOR, LOGICAL_DISK_NAME);
}

module_init(ssr_init);
module_exit(ssr_exit);


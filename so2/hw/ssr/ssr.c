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

#define NDISKS 2

struct ssr_device
{
	struct block_device *disks[NDISKS];
	struct request_queue *queue;
	struct work_struct ws;
	struct bio *cur_bio;
	struct bio *crc_bios[NDISKS];
	struct page *crc_pages[NDISKS];
	struct bio *data_bios[NDISKS];
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
	complete((struct completion*)bio->bi_private);
}

static void bi_complete(struct bio *bio, int error)
{
	/* This one runs in interrupt context */
	complete((struct completion*)bio->bi_private);
}

#define CRCS_PER_SECTOR (KERNEL_SECTOR_SIZE / sizeof(u32))
#define CRCSTART (LOGICAL_DISK_SECTORS)
#define CRCSECT(sect) (CRCSTART + ((sect) / CRCS_PER_SECTOR))
#define NSECTORS(first, last) (CRCSECT((first)) - CRCSECT((last)) + 1)


static void ssr_read_crcs(struct ssr_device *dev, int diskno)
{
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	struct bio *cur_bio = dev->cur_bio;
	struct completion event;
	struct page *page;
	sector_t first_sector;
	int num_sectors;
	sector_t first_crc_sector;
	int num_crc_sectors;

	num_sectors  = bio_sectors(cur_bio);
	first_sector = cur_bio->bi_sector;
	first_crc_sector = CRCSECT(first_sector);
	num_crc_sectors = NSECTORS(first_sector, first_sector + num_sectors);

	bio->bi_bdev = dev->disks[diskno];
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

	dev->crc_bios[diskno] = bio;
	dev->crc_pages[diskno] = page;

	//bio_put(bio);
	//__free_page(page);
}

void ssr_relay_data(struct ssr_device *dev)
{
	struct bio *bio;
	struct completion event;
	int i;

	for (i = 0; i < NDISKS; i++) {
		bio = bio_clone(dev->cur_bio, GFP_NOIO);
		bio->bi_bdev = dev->disks[i];
		init_completion(&event);
		bio->bi_private = &event;
		bio->bi_end_io = bi_complete;
		submit_bio(bio->bi_rw, bio);
		wait_for_completion(&event);
		dev->data_bios[i] = bio;
		//bio_put(bio);
	}
}

void ssr_write_sector(struct ssr_device *dev, int diskno, sector_t sect,
	char *buffer, int offset)
{
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	struct completion event;
	struct page *page;
	char *biobuf;

	bio->bi_bdev = dev->disks[diskno];
	bio->bi_sector = sect;
	bio->bi_rw = WRITE;
	init_completion(&event);
	bio->bi_private = &event;
	bio->bi_end_io = bi_complete;

	page = alloc_page(GFP_NOIO);
	bio_add_page(bio, page, KERNEL_SECTOR_SIZE, 0);
	bio->bi_vcnt = 1;
	bio->bi_idx = 0;

	biobuf = __bio_kmap_atomic(bio, 0);
	memcpy(biobuf, buffer + offset * KERNEL_SECTOR_SIZE,
		KERNEL_SECTOR_SIZE);
	__bio_kunmap_atomic(biobuf);

	submit_bio(0, bio);
	wait_for_completion(&event);

	bio_put(bio);
	__free_page(page);
}

void ssr_write_crc(struct ssr_device *dev, int diskno, sector_t sect, int nsect, char *buffer)
{
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	struct completion event;
	struct page *page;
	char *biobuf;

	bio->bi_bdev = dev->disks[diskno];
	bio->bi_sector = sect;
	bio->bi_rw = WRITE;
	init_completion(&event);
	bio->bi_private = &event;
	bio->bi_end_io = bi_complete;

	page = alloc_page(GFP_NOIO);
	bio_add_page(bio, page, nsect * KERNEL_SECTOR_SIZE, 0);
	bio->bi_vcnt = 1;
	bio->bi_idx = 0;

	biobuf = __bio_kmap_atomic(bio, 0);
	memcpy(biobuf, buffer, nsect * KERNEL_SECTOR_SIZE);
	__bio_kunmap_atomic(biobuf);

	submit_bio(0, bio);
	wait_for_completion(&event);

	bio_put(bio);
	__free_page(page);
}

int ssr_check_data(struct ssr_device *dev)
{
	int err = 0;
	struct bio *cur_bio = dev->cur_bio;

	sector_t first_sector = cur_bio->bi_sector;
	int num_sectors = bio_sectors(cur_bio);
	sector_t first_crc_sector = CRCSECT(first_sector);
	int num_crc_sectors = NSECTORS(first_sector, first_sector + num_sectors);
	int i;

	char *buf1 = __bio_kmap_atomic(dev->data_bios[0], 0);
	char *buf2 = __bio_kmap_atomic(dev->data_bios[1], 0);
	u32 *crcbuf1 = __bio_kmap_atomic(dev->crc_bios[0], 0);
	u32 *crcbuf2 = __bio_kmap_atomic(dev->crc_bios[1], 0);

	/* Ok ... this needs to be rewritten ... */

	int crcndx = first_sector % CRCS_PER_SECTOR;

	for (i = 0; i < num_sectors; i++) {
		u32 crc1 = crc32(0, buf1 + i * KERNEL_SECTOR_SIZE,
			KERNEL_SECTOR_SIZE);
		u32 crc2 = crc32(0, buf2 + i * KERNEL_SECTOR_SIZE,
			KERNEL_SECTOR_SIZE);

		if (crc1 != crcbuf1[crcndx + i] && crc2 != crcbuf2[crcndx + i]) {
			err = 1;
			printk(KERN_ERR "Unable to recover corrupt sector at: %llu\n",
				(first_sector + i));
			break;
		}

		if (crc1 != crcbuf1[crcndx + i]) {
			/*
			 * Recover from disk 2 - write sector i from disk 2
			 * and the appropriate CRC field.
			 */
			printk(KERN_WARNING)
			ssr_write_sector(dev, 1, first_sector + i, buf2, i);
			crcbuf1[crcndx + i] = crc2;
			ssr_write_crc(dev, 1, first_crc_sector,
				num_crc_sectors, (char*) crcbuf1);
		}

		if (crc2 != crcbuf2[crcndx + i]) {
			/*
			 * Recover from disk 1 - write sector i from disk 1
			 * and the appropriate CRC field.
			 */
			ssr_write_sector(dev, 0, first_sector + i, buf1, i);
			crcbuf2[crcndx + i] = crc1;
			ssr_write_crc(dev, 0, first_crc_sector,
				num_crc_sectors, (char*) crcbuf2);
		}

	}

	__bio_kunmap_atomic(buf1);
	__bio_kunmap_atomic(buf2);
	__bio_kunmap_atomic(crcbuf1);
	__bio_kunmap_atomic(crcbuf2);

	return err;
}

void ssr_update_crcs(struct ssr_device *dev)
{
	struct bio *cur_bio = dev->cur_bio;
	sector_t first_sector = cur_bio->bi_sector;
	int num_sectors = bio_sectors(cur_bio);
	sector_t first_crc_sector = CRCSECT(first_sector);
	int num_crc_sectors = NSECTORS(first_sector, first_sector + num_sectors);
	int crcndx = first_sector % CRCS_PER_SECTOR;

	u32 *crcbuf;
	char *buf;
	int s, i;

	for (i = 0; i < NDISKS; i++) {
		buf = __bio_kmap_atomic(dev->data_bios[0], 0);
		crcbuf = __bio_kmap_atomic(dev->crc_bios[0], 0);

		for (s = 0; s < num_sectors; i++) {
			u32 crc = crc32(0, buf + s * KERNEL_SECTOR_SIZE,
				KERNEL_SECTOR_SIZE);
			crcbuf[crcndx + s] = crc;
		}

		ssr_write_crc(dev, i, first_crc_sector, num_crc_sectors,
			(char*) crcbuf);
		__bio_kunmap_atomic(buf);
		__bio_kunmap_atomic(crcbuf);
	}
}

static void ssr_cleanup(struct ssr_device *dev)
{
	int i;
	for (i = 0; i < NDISKS; i++) {
		bio_put(dev->crc_bios[i]);
		__free_page(dev->crc_pages[i]);
		bio_put(dev->data_bios[i]);
	}
}

static void ssr_work_handler(struct work_struct *work)
{
	struct ssr_device *dev = container_of(work, struct ssr_device, ws);
	struct bio *cur_bio = dev->cur_bio;
	int i;

	if (!cur_bio) {
		return;
	}

	for (i = 0; i < NDISKS; i++) {
		ssr_read_crcs(dev, i);
	}
	printk(KERN_DEBUG "Finished reading CRC values\n");

	if (bio_data_dir(cur_bio)) {
		/* Propagate writes and adjust CRCs */
		//ssr_relay_data(dev);
		//ssr_update_crcs(dev);
	} else {
		/* Propagate reads and check errors */
		ssr_relay_data(dev);
		ssr_check_data(dev);
	}

	//ssr_cleanup(dev);
}

static void ssr_do_bio(struct ssr_device *dev, struct bio *bio)
{
	int i;
	dev->cur_bio = bio;
	for (i = 0; i < NDISKS; i++) {
		dev->crc_bios[i] = NULL;
	}
	schedule_work(&dev->ws);
	flush_scheduled_work();
}

static void ssr_make_request(struct request_queue *queue, struct bio *bio)
{
	struct ssr_device *dev = queue->queuedata;

	printk(KERN_DEBUG "bio: %8p dir: %lu sector: %llu sectors: %d\n",
		bio, bio_data_dir(bio), bio->bi_sector, bio_sectors(bio));

	ssr_do_bio(dev, bio);

	bio_endio(bio, 0);
}

static int open_disks(struct ssr_device *dev)
{
	int err = 0;

	dev->disks[0] = blkdev_get_by_path(PHYSICAL_DISK1_NAME,
		FMODE_READ | FMODE_WRITE | FMODE_EXCL, THIS_MODULE);
	if (dev->disks[0] == NULL) {
		printk(KERN_ERR PHYSICAL_DISK1_NAME "No such device\n");
		return -EINVAL;
	}

	dev->disks[1] = blkdev_get_by_path(PHYSICAL_DISK2_NAME,
		FMODE_READ | FMODE_WRITE | FMODE_EXCL, THIS_MODULE);
	if (dev->disks[1] == NULL) {
		printk(KERN_ERR PHYSICAL_DISK2_NAME "No such device\n");
		err = -EINVAL;
		goto err_disk;
	}

	return err;
err_disk:
	blkdev_put(dev->disks[0], FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	return err;

}

static void close_disks(struct ssr_device *dev)
{
	blkdev_put(dev->disks[0], FMODE_READ | FMODE_WRITE | FMODE_EXCL);
	blkdev_put(dev->disks[1], FMODE_READ | FMODE_WRITE | FMODE_EXCL);
}

static int ssr_create_device(struct ssr_device* dev)
{
	int err = 0;
        memset(dev, 0, sizeof(struct ssr_device));

	INIT_WORK(&dev->ws, ssr_work_handler);

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


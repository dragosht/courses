/*
 * SO2 - Block device drivers lab (#7)
 * Linux - Exercise #1, #2, #3, #6 (RAM Disk)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/genhd.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/bio.h>

MODULE_DESCRIPTION("Simple RAM Disk");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");


#define KERN_LOG_LEVEL		KERN_ALERT

//#define MY_BLOCK_MAJOR		7
#define MY_BLOCK_MAJOR		240
#define MY_BLKDEV_NAME		"mybdev"
#define MY_BLOCK_MINORS		1
#define NR_SECTORS		128

#define KERNEL_SECTOR_SIZE	512

/* TODO 6: use bios for read/write requests */
//#define USE_BIO_TRANSFER	0
#define USE_BIO_TRANSFER	1


static struct my_block_dev {
	spinlock_t lock;
	struct request_queue *queue;
	struct gendisk *gd;
	u8 *data;
	size_t size;
} g_dev;

static int my_block_open(struct block_device *bdev, fmode_t mode)
{
	return 0;
}

static void my_block_release(struct gendisk *gd, fmode_t mode)
{
}

static const struct block_device_operations my_block_ops = {
	.owner = THIS_MODULE,
	.open = my_block_open,
	.release = my_block_release
};

static void my_block_transfer(struct my_block_dev *dev, sector_t sector,
		unsigned long len, char *buffer, int dir)
{
	unsigned long offset = sector * KERNEL_SECTOR_SIZE;

	/* check for read/write beyond end of block device */
	if ((offset + len) > dev->size)
		return;

	/* TODO 3: read/write to dev buffer depending on dir */

	if (dir) {
		memcpy(dev->data + offset, buffer, len);
	} else {
		memcpy(buffer, dev->data + offset, len);
	}
}

/* to transfer data using bio structures enable USE_BIO_TRANFER */
#if USE_BIO_TRANSFER == 1

/*
 * Suggested solution
 */

static unsigned long my_xfer_request(struct my_block_dev *dev,
		struct request *req)
{
	unsigned long nbytes = 0;
	struct bio_vec *bvec;
	struct bio *bio;
	size_t i;

	printk(KERN_ALERT "begin segment walk\n");
	__rq_for_each_bio(bio, req) {
		sector_t start_sector = bio->bi_sector;
		bio_for_each_segment(bvec, bio, i) {
			char *buffer = __bio_kmap_atomic(bio, i);
			size_t len = bvec->bv_len;

			printk(KERN_ALERT "rq: %8p, bio: %8p, bvec: %d, len: %d\n", req, bio, i, len);
			printk(KERN_ALERT "bio: %p, sector: %llu, vcnt: %d, idx: %d\n", bio, start_sector, bio->bi_vcnt, bio->bi_idx+i);
			printk(KERN_ALERT "bio: %p, dir: %s\n", bio,
				(bio_data_dir(bio) == WRITE) ? "write" : "read");
			my_block_transfer(dev, start_sector, len,
				buffer, bio_data_dir(bio) == WRITE);

			__bio_kunmap_atomic(buffer);

			nbytes += len;
			start_sector += len / KERNEL_SECTOR_SIZE;
		}
	}
	printk(KERN_ALERT "complete segment walk\n");

	return nbytes;
}

/*
static unsigned long my_xfer_request(struct my_block_dev *dev,
		struct request *req)
{
	unsigned long nbytes = 0;
	struct bio_vec *bvec;
	struct req_iterator iter;
	sector_t start = 0;

	// TODO 6: iterate segments
	// TODO 6: copy bio data to device buffer

	printk(KERN_DEBUG "walking bio segments:\n");
	rq_for_each_segment(bvec, req, iter) {
		char *buffer = __bio_kmap_atomic(iter.bio, iter.i);

		if (iter.i == 0) {
			start = iter.bio->bi_sector;
		}

		printk(KERN_DEBUG "rq: %8p, bio: %8p, bvec: %d len: %d "
			"sector: %llu vcnt: %d dir: %s\n",
			req, iter.bio, iter.i, bvec->bv_len,
			start, iter.bio->bi_vcnt,
			(bio_data_dir(iter.bio) == WRITE) ? "write" : "read");

		__bio_kunmap_atomic(buffer);

		nbytes += bvec->bv_len;
		start += bvec->bv_len / KERNEL_SECTOR_SIZE;
	}
	printk(KERN_ALERT "complete segment walk\n");

	return nbytes;
}
*/
#endif

static void my_block_request(struct request_queue *q)
{
	struct request *rq;
	struct my_block_dev *dev = q->queuedata;

	while (1) {

		/* TODO 2: fetch request */
		rq = blk_fetch_request(q);
		if (rq == NULL) {
			break;
		}

		/* TODO 2: check fs request */
		if (rq->cmd_type != REQ_TYPE_FS) {
			printk (KERN_NOTICE "Skip non-fs request\n");
			__blk_end_request_all(rq, -EIO);
			continue;
		}

		/* TODO 2: print request information */
		printk(KERN_LOG_LEVEL
			"request received: pos=%llu bytes=%u "
			"cur_bytes=%u dir=%c\n",
			(unsigned long long) blk_rq_pos(rq),
			blk_rq_bytes(rq),
			blk_rq_cur_bytes(rq),
			rq_data_dir(rq) ? 'W' : 'R');

#if USE_BIO_TRANSFER == 1
		/* TODO 6: process the request by calling my_xfer_request */
		my_xfer_request(dev, rq);
#else
		/* TODO 3: process the request by calling my_block_transfer */
		my_block_transfer(dev, blk_rq_pos(rq), blk_rq_cur_bytes(rq),
				  rq->buffer, rq_data_dir(rq));
#endif

		/* TODO 2: end request successfully */
		__blk_end_request_all(rq, 0);
	}
}

static int create_block_device(struct my_block_dev *dev)
{
	int err;

	dev->size = NR_SECTORS * KERNEL_SECTOR_SIZE;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL) {
		printk(KERN_ERR "vmalloc: out of memory\n");
		err = -ENOMEM;
		goto out_vmalloc;
	}

	/* initialize the I/O queue */
	spin_lock_init(&dev->lock);
	dev->queue = blk_init_queue(my_block_request, &dev->lock);
	if (dev->queue == NULL) {
		printk(KERN_ERR "blk_init_queue: out of memory\n");
		err = -ENOMEM;
		goto out_blk_init;
	}
	blk_queue_logical_block_size(dev->queue, KERNEL_SECTOR_SIZE);
	dev->queue->queuedata = dev;

	/* initialize the gendisk structure */
	dev->gd = alloc_disk(MY_BLOCK_MINORS);
	if (!dev->gd) {
		printk(KERN_ERR "alloc_disk: failure\n");
		err = -ENOMEM;
		goto out_alloc_disk;
	}

	dev->gd->major = MY_BLOCK_MAJOR;
	dev->gd->first_minor = 0;
	dev->gd->fops = &my_block_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf(dev->gd->disk_name, DISK_NAME_LEN, "myblock");
	set_capacity(dev->gd, NR_SECTORS);

	add_disk(dev->gd);

	return 0;

out_alloc_disk:
	blk_cleanup_queue(dev->queue);
out_blk_init:
	vfree(dev->data);
out_vmalloc:
	return err;
}

static int __init my_block_init(void)
{
	int err = 0;

	/* TODO 1: register block device */
	err = register_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
	if (err) {
		printk(KERN_ERR "unable to register mybdev block device\n");
		return -EBUSY;
	}

	/* TODO 2: uncomment - create block device */
	err = create_block_device(&g_dev);
	if (err < 0)
		goto out;

	return 0;

out:
	/* TODO 1: unregister block device in case of an error */
	unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
	return err;
}

static void delete_block_device(struct my_block_dev *dev)
{
	if (dev->gd) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}
	if (dev->queue)
		blk_cleanup_queue(dev->queue);
	if (dev->data)
		vfree(dev->data);
}

static void __exit my_block_exit(void)
{
	/* TODO 2: uncomment - cleanup block device */
	delete_block_device(&g_dev);

	/* TODO 1: unregister block device */
	unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
}

module_init(my_block_init);
module_exit(my_block_exit);


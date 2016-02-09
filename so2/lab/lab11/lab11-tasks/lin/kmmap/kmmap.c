/*
 * SO2 - Memory Mapping Lab(#11)
 *
 * Exercise #1: memory mapping using kmalloc'd kernel areas
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/highmem.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "../test/mmap-test.h"

MODULE_DESCRIPTION("simple mmap driver");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("Dual BSD/GPL");

#define MY_MAJOR	42
/* how many pages do we actually kmalloc */
#define NPAGES		16

/* character device basic structure */
static struct cdev mmap_cdev;

/* pointer to kmalloc'd area */
static void *kmalloc_ptr;

/* pointer to the kmalloc'd area, rounded up to a page boundary */
static char *kmalloc_area;

static int my_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int my_read(struct file *file, char __user *user_buffer,
		size_t size, loff_t *offset)
{
	/* TODO3: check size doesn't exceed our mapped area size */

	/* TODO3: copy from mapped area to user buffer */

	return 0;
}

static int my_write(struct file *file, const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	/* TODO3: check size doesn't exceed our mapped area size */

	/* TODO3: copy from user buffer to mapped area */

	return 0;
}

static int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;
	long length = vma->vm_end - vma->vm_start;
	unsigned long phys_pgoff;

	/* do not map more than we can */
	if (length > NPAGES * PAGE_SIZE)
		return -EIO;

	//physical page frame number
	phys_pgoff = virt_to_phys((void*)kmalloc_area) >> PAGE_SHIFT;

	/* TODO1: map the whole physically contiguous area in one piece */
	ret = remap_pfn_range(vma, vma->vm_start, phys_pgoff,
				length, vma->vm_page_prot);
	if (ret < 0) {
		printk(KERN_ERR "Unable to map kmalloc area\n");;
		return -EIO;
	}

	return 0;
}

static const struct file_operations mmap_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.mmap = my_mmap,
	.read = my_read,
	.write = my_write,
};

static int my_seq_show(struct seq_file *seq, void *v)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma_iterator;
	unsigned long total = 0;

	/* TODO4: Get current process' mm_struct */

	/* TODO4: Iterate through all memory mappings and update total */

	/* TODO4: Release mm_struct */

	/* TODO4: print the total (%lu) to the seq_file */
	return 0;
}

static int my_seq_open(struct inode *inode, struct file *file)
{
	/* TODO4: use my_seq_show as display function */
	return -ENOSYS;
}

static const struct file_operations my_proc_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int __init my_init(void)
{
	int ret = 0;
	int i;
	struct proc_dir_entry *entry;

	/* TODO4: create proc entry PROC_ENTRY_NAME */
	/* TODO4: initialize proc_fops with use my_proc_file_ops */

	ret = register_chrdev_region(MKDEV(MY_MAJOR, 0), 1, "mymap");
	if (ret < 0) {
		printk(KERN_ERR "could not register region\n");
		goto out_no_chrdev;
	}

	/* TODO1: allocate NPAGES+1 pages using kmalloc */
	kmalloc_ptr = kmalloc((NPAGES+1) * PAGE_SIZE, GFP_KERNEL);
	if (!kmalloc_ptr) {
		printk(KERN_ALERT "Unable to allocate kernel memory\n");
		goto out_unreg;
	}

	/* TODO1: round kmalloc_ptr to nearest page start address */
	kmalloc_area = (char *)((((unsigned long)kmalloc_ptr) + PAGE_SIZE - 1) & PAGE_MASK);

	for (i = 0; i < NPAGES * PAGE_SIZE; i += PAGE_SIZE) {
		SetPageReserved(virt_to_page(((unsigned long)kmalloc_area) + i));
	}

	/* TODO1: write data in each page */
	for (i = 0; i < NPAGES; i++) {
		kmalloc_area[i*PAGE_SIZE    ] = 0xaa;
		kmalloc_area[i*PAGE_SIZE + 1] = 0xbb;
		kmalloc_area[i*PAGE_SIZE + 2] = 0xcc;
		kmalloc_area[i*PAGE_SIZE + 3] = 0xdd;
	}

	/* Init device. */
	cdev_init(&mmap_cdev, &mmap_fops);
	ret = cdev_add(&mmap_cdev, MKDEV(MY_MAJOR, 0), 1);
	if (ret < 0) {
		printk(KERN_ERR "could not add device\n");
		goto out_kfree;
	}

	return 0;

out_kfree:
	kfree(kmalloc_ptr);
out_unreg:
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);
out_no_chrdev:
	/* TODO4: remove proc entry PROC_ENTRY_NAME */
out:
	return ret;
}

static void __exit my_exit(void)
{
	int i;

	cdev_del(&mmap_cdev);

	/* TODO1: clear reservation on pages and free mem. */
	for (i = 0; i < NPAGES * PAGE_SIZE; i += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(((unsigned long)kmalloc_area) + i));
	}

	kfree(kmalloc_ptr);

	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);

	/* TODO4: remove proc entry PROC_ENTRY_NAME */
}

module_init(my_init);
module_exit(my_exit);


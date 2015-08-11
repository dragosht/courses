/******************************************************************************
 * File Name:   memory.c
 * Program:     Linux Driver Primer Class
 * Author:      Saleem Yamani
 * Purpose:     Memory Driver which stores the last character sent to it.
 *
 *******************************************************************************/

/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/ctype.h>

#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations memory_fops =
{
  read: memory_read,
  write: memory_write,
  open: memory_open,
  release: memory_release
};

/* Declaration of the init and exit functions */
module_init(memory_init);
module_exit(memory_exit);

/* Global variables of the driver */
/* Major number */
int memory_major = 60;

/* Buffer to store data */
char *memory_buffer;



/* Memory Init Module */
int memory_init(void)
{
  int result;
  printk(KERN_ALERT "memory_init\n");

  /* Registering device */
  result = register_chrdev(memory_major, "memory", &memory_fops);
  if (result < 0) {
    printk(
      "<1>memory: cannot obtain major number %d\n", memory_major);
    return result;
  }

  /* Allocating memory for the buffer */
  memory_buffer = kmalloc(1, GFP_KERNEL);

  if(!memory_buffer)
  {
    memory_exit();
    return(-ENOMEM);
  }
  else
  {
    memset(memory_buffer, 0, 1);
    printk("<1>Inserting memory module\n");
    return 0;
  }
}



/* Memory Exit */
void memory_exit(void)
{

  printk("<1>Removing memory module\n");

  /* Freeing the major number */
  unregister_chrdev(memory_major, "memory");

  /* Freeing buffer memory */
  if (memory_buffer) {
    kfree(memory_buffer);
  }
}

/* Memory Open */
int memory_open(struct inode *inode, struct file *filp)
{
  printk(KERN_ALERT "memory_open\n");

  /* Success */
  return 0;
}

/* Memory Release */
int memory_release(struct inode *inode, struct file *filp)
{
  printk(KERN_ALERT "memory_release\n");

  /* Success */
  return 0;
}

/* Memory Read */
ssize_t memory_read(struct file *filp, char *buf,
                    size_t count, loff_t *f_pos)
{

  printk(KERN_ALERT "memory_read\n");
  /* Transfering data to user space */
  copy_to_user(buf,memory_buffer,1);

  /* Changing reading position as best suits */
  if (*f_pos == 0)
  {
    *f_pos += 1;
    return 1;
  }
  else
  {
    return 0;
  }
}

/* Memory Write */
ssize_t memory_write( struct file *filp, char *buf,
                      size_t count, loff_t *f_pos)
{
  char *tmp;

  tmp = buf + count - 1;
  copy_from_user(memory_buffer, tmp, 1);

  memory_buffer[0] = toupper(memory_buffer[0]);

  printk(KERN_ALERT "memory_write\n");

  return 1;
}

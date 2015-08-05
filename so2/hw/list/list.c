#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/slab.h>
#include <linux/list.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/string.h>
#include <linux/ctype.h>

#include <asm/uaccess.h>

MODULE_DESCRIPTION("List Data Structure in procfs");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define procfs_dir_name		"list"
#define procfs_file_read	"preview"
#define procfs_file_write	"management"

static struct proc_dir_entry *proc_list;
static struct proc_dir_entry *proc_list_read;
static struct proc_dir_entry *proc_list_write;

#define PROCFS_MAX_SIZE		1024

#define LIST_CMD_ADD_FIRST  "addf"
#define LIST_CMD_ADD_END    "adde"
#define LIST_CMD_DEL_FIRST  "delf"
#define LIST_CMD_DEL_ALL    "dela"

#define LIST_ADD_FIRST 0
#define LIST_ADD_END 1
#define LIST_DEL_FIRST 2
#define LIST_DEL_ALL 3

/*
 * TODO: add synchronization!
 */
struct string_node {
	char *string;
	struct list_head list;
};

static struct list_head head;

static int list_proc_show(struct seq_file *m, void *v)
{
	struct list_head *p;
	struct string_node *sn;

	list_for_each(p, &head) {
		sn = list_entry(p, struct string_node, list);
		seq_printf(m, "%s\n", sn->string);
	}

	return 0;
}

static int list_read_open(struct inode *inode, struct  file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_write_open(struct inode *inode, struct  file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_string_add(char *param, int flags)
{
	struct string_node *sn;

	sn = kmalloc(sizeof(struct string_node), GFP_KERNEL);
	if (!sn) {
		return -ENOMEM;
	}

	sn->string = kstrdup(param, GFP_KERNEL);
	if (!sn->string) {
		goto err_no_str;
	}

	if (flags == LIST_ADD_FIRST) {
		list_add(&sn->list, &head);
	} else {
		list_add_tail(&sn->list, &head);
	}

	return 0;

err_no_str:
	kfree(sn);
	return -ENOMEM;
}

static int list_string_del(char *param, int flags)
{
	struct list_head *p, *q;
	struct string_node *sn;

	list_for_each_safe(p, q, &head) {
		sn = list_entry(p, struct string_node, list);
		if (!strcmp(param, sn->string)) {
			list_del(p);
			kfree(sn->string);
			kfree(sn);

			if (flags == LIST_DEL_FIRST) {
				return 0;
			}
		}
	}

	return 0;
}

static void list_clear(void)
{
	struct list_head *p, *q;
	struct string_node *sn;

	list_for_each_safe(p, q, &head) {
		sn = list_entry(p, struct string_node, list);
		list_del(p);
		kfree(sn->string);
		kfree(sn);
	}
}

static int list_do_write_cmd(char *buffer)
{
	char *cmdstr;
	char *param;
	int   cmdlen;

	cmdstr = strim(buffer);
	param = cmdstr;
	cmdlen = 0;

	while (param && isalpha(*param)) {
		param++;
		cmdlen++;
	}

	param = strim(param);
	if (param == NULL) {
		pr_warn("list: null parameter\n");
		return -EINVAL;
	}

	if (!strncmp(cmdstr, LIST_CMD_ADD_FIRST, cmdlen)) {
		pr_debug("list: adding first  %s\n", param);
		return list_string_add(param, LIST_ADD_FIRST);
	} else if (!strncmp(cmdstr, LIST_CMD_ADD_END, cmdlen)) {
		pr_debug("list: adding end: %s\n", param);
		return list_string_add(param, LIST_ADD_END);
	} else if (!strncmp(cmdstr, LIST_CMD_DEL_FIRST, cmdlen)) {
		pr_debug("list: del first: %s\n", param);
		return list_string_del(param, LIST_DEL_FIRST);
	} else if (!strncmp(cmdstr, LIST_CMD_DEL_ALL, cmdlen)) {
		pr_debug("list: del all: %s\n", param);
		return list_string_del(param, LIST_DEL_ALL);
	} else {
		return -EINVAL;
	}
}

static ssize_t list_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *offs)
{
	char local_buffer[PROCFS_MAX_SIZE];
	int local_buffer_size;

	local_buffer_size = count;
	if (local_buffer_size > PROCFS_MAX_SIZE) {
		local_buffer_size = PROCFS_MAX_SIZE;
	}

	memset(local_buffer, 0, PROCFS_MAX_SIZE);
	if (copy_from_user(local_buffer, buffer, local_buffer_size)) {
		return -EFAULT;
	}

	if (list_do_write_cmd(local_buffer)) {
		return -EINVAL;
	}

	return local_buffer_size;
}

static const struct file_operations r_fops = {
	.owner		= THIS_MODULE,
	.open		= list_read_open,
	.read		= seq_read,
	.release	= single_release,
};

static const struct file_operations w_fops = {
	.owner		= THIS_MODULE,
	.open		= list_write_open,
	.write		= list_write,
	.release	= single_release,
};

static int list_init(void)
{
	INIT_LIST_HEAD(&head);

	proc_list = proc_mkdir(procfs_dir_name, NULL);
	if (!proc_list)
		return -ENOMEM;

	proc_list_read = proc_create(procfs_file_read, 0, proc_list, &r_fops);
	if (!proc_list_read)
		goto proc_list_cleanup;

	proc_list_write = proc_create(procfs_file_write, 0, proc_list, &w_fops);
	if (!proc_list_write)
		goto proc_list_cleanup;

	return 0;

proc_list_cleanup:
	list_clear();
	proc_remove(proc_list);
	return -ENOMEM;
}

static void list_exit(void)
{
	list_clear();
	proc_remove(proc_list);
}


module_init(list_init);

module_exit(list_exit);

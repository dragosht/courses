#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dragos Tarcatu");

static short int    cmdmod_short = 0;
static int          cmdmod_int = 0;
static long int     cmdmod_long = 0;
static char         *cmdmod_string = "Default";
static int          cmdmod_int_array[2] = { -1, -1 };
static int          cmdmod_arr_argc = 0;

/*
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

module_param(cmdmod_short, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(cmdmod_short, "short");
module_param(cmdmod_int, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cmdmod_int, "int");
module_param(cmdmod_long, long, S_IRUSR);
MODULE_PARM_DESC(cmdmod_long, "long");
module_param(cmdmod_string, charp, 0000);
MODULE_PARM_DESC(cmdmod_string, "string");

/*
 * module_param_array(name, type, num, perm);
 * The first param is the parameter's (in this case the array's) name
 * The second param is the data type of the elements of the array
 * The third argument is a pointer to the variable that will store the number
 * of elements of the array initialized by the user at module loading time
 * The fourth argument is the permission bits
 */
module_param_array(cmdmod_int_array, int, &cmdmod_arr_argc, 0000);
MODULE_PARM_DESC(cmdmod_int_array, "An array of integers");

static int __init hello_5_init(void)
{
	int i;
	printk(KERN_INFO "cmdmod");
	printk(KERN_INFO "cmdmod_short: %hd\n", cmdmod_short);
	printk(KERN_INFO "cmdmod_int: %d\n", cmdmod_int);
	printk(KERN_INFO "cmdmod_long: %ld\n", cmdmod_long);
	printk(KERN_INFO "cmdmod_string: %s\n", cmdmod_string);

        for (i = 0; i < (sizeof cmdmod_int_array / sizeof (int)); i++)
	{
		printk(KERN_INFO "cmdmod_int_array[%d] = %d\n", i, cmdmod_int_array[i]);
	}
	printk(KERN_INFO "Received %d arguments for cmdmod_int_array.\n", cmdmod_arr_argc);
	return 0;
}

static void __exit hello_5_exit(void)
{
	printk(KERN_INFO "Goodbye, world 5\n");
}

module_init(hello_5_init);
module_exit(hello_5_exit);


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Oops");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define OP_READ 0
#define OP_WRITE 1
#define OP_OOPS OP_WRITE

static int myoops_init(void)
{
    int *a;
    a = (int *) 0x00001234;
#if OP_OOPS == OP_WRITE
    *a = 3;
#elif OP_OOPS == OP_READ
    printk (KERN_ALERT "value = %d\n", *a);
#else
#error "Unknown op for oops!"
#endif
    return 0;
}

static void myoops_exit(void)
{
    printk(KERN_DEBUG "Bye!\n");
}

module_init(myoops_init);
module_exit(myoops_exit);


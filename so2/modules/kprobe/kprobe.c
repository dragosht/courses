#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>

MODULE_DESCRIPTION("Dummy");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

static struct kprobe kp;

int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    printk(KERN_DEBUG "handler_pre\n");
    return 0;
}

int handler_post(struct kprobe *p, struct pt_regs *regs)
{
    printk(KERN_DEBUG "handler_post\n");
    return 0;
}

int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk(KERN_DEBUG "handler_fault\n");
    return 0;
}

static int kprobe_init(void)
{
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_post;
    kp.addr = (kprobe_opcode_t*) ;

    if ((ret = register_kprobe(&kp) < 0)) {
        printk("register_kprobe failed, returned %d\n", ret);
        return -1;
    }
    printk("kprobe registered\n");
    return 0;
}

static void kprobe_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_DEBUG "Bye!\n");
}

module_init(kprobe_init);
module_exit(kprobe_exit);


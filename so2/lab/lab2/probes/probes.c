#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

MODULE_DESCRIPTION("Probes module");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_WARNING

/* KProbe stuff ... */

static struct kprobe kp;

static
int kprobe_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    printk(KERN_INFO"kprobe pre handler\n");
    return 0;
}

static
void kprobe_handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    printk(KERN_INFO"kprobe post handler\n");
}

static
int kprobe_handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk(KERN_INFO"kprobe fault handler\n");
    return 0;
}

static
int kprobe_init(void)
{
    kp.pre_handler = kprobe_handler_pre;
    kp.post_handler = kprobe_handler_post;
    kp.fault_handler = kprobe_handler_fault;
    kp.addr = (kprobe_opcode_t *) kallsyms_lookup_name("do_execve");
    if (!kp.addr) {
        printk("Couldn't find %s to plant kprobe\n", "do_execve");
        return -1;
    }

    return register_kprobe(&kp);
}

static
void kprobe_release(void)
{
    unregister_kprobe(&kp);
}

/* JProbe stuff ... */

static struct jprobe jp;

static
int jprobe_execve(char *filename,
                  char __user * __user *argv,
                  char __user * __user *envp)
{
    printk("jprobe: execve %s %s\n", filename, current->comm);
    jprobe_return();
    return 0;
}

static
int jprobe_init(void)
{
    jp.entry = (kprobe_opcode_t*) jprobe_execve;
    jp.kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("do_execve");
    if (!jp.kp.addr) {
        printk("Couldn't find %s to plant jprobe\n", "do_execve");
        return -1;
    }
    return register_jprobe(&jp);
}

static
void jprobe_release(void)
{
    unregister_jprobe(&jp);
}

/* KRetProbe stuff ... */

static struct kretprobe kretp;

static
int kretprobe_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    printk("kretprobe: handler\n");
        return 0;
}

static
int kretprobe_init(void)
{
    printk(KERN_INFO"probes: initializing kprobe\n");
    kretp.handler = kretprobe_handler;
    kretp.kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("do_execve");
    if (!kretp.kp.addr) {
        printk("Couldn't find %s to plant kretprobe\n", "do_execve");
        return -1;
    }
    return register_kretprobe(&kretp);
}

static
void kretprobe_release(void)
{
    unregister_kretprobe(&kretp);
}


int probe_init(void)
{
    kprobe_init();
    jprobe_init();
    kretprobe_init();
    return 0;
}

void probe_exit(void)
{
    kprobe_release();
    jprobe_release();
    kretprobe_release();
}


module_init(probe_init);
module_exit(probe_exit);


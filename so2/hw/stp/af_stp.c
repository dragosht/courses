#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/fs.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <linux/hashtable.h>
#include <linux/list.h>

#include "stp.h"

MODULE_DESCRIPTION("STP (SO2 Transport Protocol)");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");

#define STPLOG_PREFIX "[STP]: "

DEFINE_HASHTABLE(stp_hbind, 16);

struct stp_bind_entry {
	__be16 port;
	struct sock *sk;
	struct hlist_node hnode;
};

struct stp_sock {
	struct sock	*sk;
	__be16		bport;
};

static struct proto stp_proto = {
	.name	= "STP",
	.owner	= THIS_MODULE,
	.obj_size = sizeof(struct stp_sock),
};

static int stp_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct stp_sock *ssk = (struct stp_sock *) sk;

	printk(KERN_DEBUG "Releasing socket: %p\n", sock);

	if (!sk)
		return 0;

	/*
	if (ssk->bport) {
		// This was bound to some port. Remove this entry from the hash.
		printk(KERN_DEBUG "Removing socket entry from hash\n");
		struct stp_bind_entry *sbe;
		struct hlist_node *tmp;

		hash_for_each_possible_safe(stp_hbind, sbe, tmp, hnode, ssk->bport) {
			if (sbe->port == ssk->bport) {
				hash_del(&sbe->hnode);
				break;
			}
		}
	}
	*/

	sock->sk = NULL;
	skb_queue_purge(&sk->sk_receive_queue);
	sk_refcnt_debug_release(sk);

	sock_put(sk);

	return 0;
}




static int stp_bind(struct socket *sock, struct sockaddr *uaddr,
			int addr_len)
{
	struct sockaddr_stp *addr = (struct sockaddr_stp *) uaddr;
	struct sock *sk = sock->sk;
	struct stp_sock *ssk = (struct stp_sock *) sk;

	struct stp_bind_entry *sbe;

	/* Check for proper address size */
	if (addr_len != sizeof(struct sockaddr_stp))
		return -EINVAL;
	if (addr->sas_family != AF_STP)
		return -EINVAL;

	if (addr->sas_ifindex) {
		if (!dev_get_by_index(sock_net(sk), addr->sas_ifindex))
			return -ENODEV;
	}

	hash_for_each_possible(stp_hbind, sbe, hnode, addr->sas_port) {
		if (sbe->port == addr->sas_port) {
			return -EADDRINUSE;
		}
	}

	sbe = kmalloc(sizeof(struct stp_bind_entry), GFP_KERNEL);
	if (!sbe)
		return -ENOMEM;


	sbe->port = addr->sas_port;
	ssk->bport = addr->sas_port;
	//This probably also needs a reference count increase ...
	sbe->sk = sk;
	hash_add(stp_hbind, &sbe->hnode, sbe->port);

	printk(KERN_DEBUG "Bind socket: %p to port: %d\n", ssk, addr->sas_port);

	return 0;
}

int stp_connect(struct socket *sock, struct sockaddr *uaddr,
		int addr_len, int flags)
{
	return 0;
}

unsigned int stp_poll(struct file *file, struct socket *sock,
			struct poll_table_struct *wait)
{
	return datagram_poll(file, sock, wait);
}

int stp_sendmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t size)
{
	return 0;
}

int stp_recvmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t size, int flags)
{
	return 0;
}

static const struct proto_ops stp_ops = {
	.family		= PF_STP,
	.owner		= THIS_MODULE,
	.release	= stp_release,
	.bind		= stp_bind,
	.connect	= stp_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= sock_no_accept,
	.getname	= sock_no_getname,
	.poll		= stp_poll,
	.ioctl		= sock_no_ioctl,
	.listen		= sock_no_listen,
	.shutdown	= sock_no_shutdown,
	.setsockopt	= sock_no_setsockopt,
	.getsockopt	= sock_no_getsockopt,
	.sendmsg	= stp_sendmsg,
	.recvmsg	= stp_recvmsg,
	.mmap		= sock_no_mmap,
	.sendpage	= sock_no_sendpage,
};




static void stp_sock_destruct(struct sock *sk)
{
	skb_queue_purge(&sk->sk_error_queue);
	sk_refcnt_debug_dec(sk);
}

/* Create an STP socket */
static int stp_create(struct net *net, struct socket *sock,
		int protocol, int kern)
{
	struct sock *sk;
	struct stp_sock *ssk;
	int err;

	printk(KERN_DEBUG "Creating socket: %p\n", sock);

	/*
	 * No support for anything other than datagrams ...
	 * And only one protocol.
	 */
	if (sock->type != SOCK_DGRAM || protocol != 0)
		return -ESOCKTNOSUPPORT;

	sock->ops = &stp_ops;
	sock->state = SS_UNCONNECTED;

	sk = sk_alloc(net, PF_STP, GFP_KERNEL, &stp_proto);
	if (!sk) {
		printk(KERN_ALERT STPLOG_PREFIX "Error allocating sk\n");
		return -ENOBUFS;
	}

	ssk = (struct stp_sock*) sk;

	sock_init_data(sock, sk);
	sk->sk_destruct = stp_sock_destruct;
	sk->sk_protocol = protocol;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;
	sk->sk_family = PF_STP;

	sk_refcnt_debug_inc(sk);

	//ssk->bport = 0;

	return 0;
}

static const struct net_proto_family stp_family_ops = {
	.family	= PF_STP,
	.create = stp_create,
	.owner	= THIS_MODULE,
};



static int stp_seq_show(struct seq_file *seq, void *v)
{
	return 0;
}

static int stp_seq_open(struct inode *inode, struct file *file)
{
	return single_open_net(inode, file, stp_seq_show);
}

struct file_operations stp_seq_fops = {
	.owner		= THIS_MODULE,
	.open		= stp_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release_net,
};

static __net_init int stp_init_net(struct net *net)
{
	if (!proc_create(STP_PROC_NET_FILENAME, S_IRUGO, net->proc_net,
			&stp_seq_fops))
		return -ENOMEM;
	return 0;
}

static __net_exit void stp_exit_net(struct net *net)
{
	remove_proc_entry(STP_PROC_NET_FILENAME, net->proc_net);
}

static __net_initdata struct pernet_operations stp_proc_ops = {
	.init = stp_init_net,
	.exit = stp_exit_net,
};

int __init stp_init(void)
{
	int ret;

	ret = register_pernet_subsys(&stp_proc_ops);
	if (ret != 0) {
		printk(KERN_ALERT STPLOG_PREFIX
			"Unable to create /proc/net/ entry");
		goto out;
	}

	ret = proto_register(&stp_proto, 0);
	if (ret != 0) {
		printk(KERN_ALERT STPLOG_PREFIX "Unable to register protocol\n");
		goto out_clear_proc;
	}

	sock_register(&stp_family_ops);

	return 0;

out_clear_proc:
	unregister_pernet_subsys(&stp_proc_ops);
out:
	return ret;
}

void __exit stp_exit(void)
{
	sock_unregister(PF_STP);
	proto_unregister(&stp_proto);
	unregister_pernet_subsys(&stp_proc_ops);
}

module_init(stp_init);
module_exit(stp_exit);


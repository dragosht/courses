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

struct stp_stats {
	int num_rx_pkts;
	int num_hdr_err;
	int num_csum_err;
	int num_no_sock;
	int num_no_buffs;
	int num_tx_pkts;
};

struct stp_sock {
	/*
	Wasted a lot of time here because of this:
	struct sock *sk;
	*/
	struct sock		sk;
	spinlock_t		lock;

	int			bnd_ifindex;
	__be16			bnd_port;

	struct hlist_node	hnode;
};

static struct stp_sock *stp_sk(struct sock *sk)
{
	return (struct stp_sock*)sk;
}

static struct stp_stats stp_stats = {0};

/* STP sockets by port number mappings */
DEFINE_HASHTABLE (stp_bind_socks, 16);

static void stp_bind_socks_clear(void)
{
	int bkt;
	struct hlist_node *tmp;
	struct stp_sock *ssk;

	hash_for_each_safe(stp_bind_socks, bkt, tmp, ssk, hnode) {
		hash_del(&ssk->hnode);
	}
}

static void stp_bind_socks_del(__be16 port)
{
	int bkt;
	struct hlist_node *tmp;
	struct stp_sock *ssk;

	hash_for_each_safe(stp_bind_socks, bkt, tmp, ssk, hnode) {
		if (ssk->bnd_port == port)
			hash_del(&ssk->hnode);
	}
}

static int stp_rcv(struct sk_buff *skb, struct net_device *dev,
		struct packet_type *pt, struct net_device *orig_dev)
{
	printk(KERN_ALERT "stp_rcv\n");
	return 0;
}

static struct packet_type stp_packet_type = {
	.type = ntohs(ETH_P_STP),
	.func = stp_rcv,
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

	if (!sk)
		return 0;

	if (ssk->bnd_port) {
		spin_lock(&ssk->lock);
		stp_bind_socks_del(ssk->bnd_port);
		ssk->bnd_port = 0;
		spin_unlock(&ssk->lock);
	}

	synchronize_net();
	sock_orphan(sk);

	sock->sk = NULL;

	skb_queue_purge(&sk->sk_receive_queue);
	sk_refcnt_debug_release(sk);
	sock_put(sk);

	return 0;
}

static int stp_do_bind(struct sock *sk, struct net_device *dev,
		struct sockaddr_stp *saddr)
{
	struct stp_sock *ssk;
	struct stp_sock *bsk;
	int err = 0;

	lock_sock(sk);

	ssk = stp_sk(sk);
	spin_lock(&ssk->lock);

	hash_for_each_possible(stp_bind_socks, bsk, hnode, saddr->sas_port) {
		err = -EBUSY;
		goto out_unlock;
	}

	ssk->bnd_ifindex = saddr->sas_ifindex;
	ssk->bnd_port = saddr->sas_port;

	hash_add(stp_bind_socks, &ssk->hnode, saddr->sas_port);

out_unlock:
	spin_unlock(&ssk->lock);
	release_sock(sk);

	return err;
}

static int stp_bind(struct socket *sock, struct sockaddr *uaddr,
			int addr_len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_stp *saddr = (struct sockaddr_stp *)uaddr;
	struct net_device *dev = NULL;
	int err = 0;

	printk(KERN_ALERT "%s\n", __func__);
	if (addr_len < sizeof(struct sockaddr_stp))
		return -EINVAL;
	if (saddr->sas_family != AF_STP)
		return -EINVAL;

	if (saddr->sas_ifindex) {
		dev = dev_get_by_index(sock_net(sk), saddr->sas_ifindex);
		if (dev == NULL) {
			err = -ENODEV;
			goto out;
		}

		if (!(dev->flags & IFF_UP)) {
			err = -ENETDOWN;
			dev_put(dev);
			goto out;
		}
	}

	err = stp_do_bind(sk, dev, saddr);

out:
	return err;
}


int stp_connect(struct socket *sock, struct sockaddr *uaddr,
		int addr_len, int flags)
{
	printk(KERN_ALERT "%s\n", __func__);
	return 0;
}

int stp_sendmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t len)
{
	//struct sock *sk = sock->sk;
	//struct stp_sock *ssk = stp_sk(sk);
	//struct sk_buff *skb;
	//struct net_device *dev;
	printk(KERN_ALERT "%s\n", __func__);

	//dev = dev_get_by_index(sock_net(sk), ssk->baddr->sas_ifindex);

	return 0;
}

int stp_recvmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t size, int flags)
{
	printk(KERN_ALERT "%s\n", __func__);
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
	.poll		= datagram_poll,
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
	__skb_queue_purge(&sk->sk_receive_queue);
	__skb_queue_purge(&sk->sk_error_queue);

	WARN_ON(atomic_read(&sk->sk_rmem_alloc));
	WARN_ON(atomic_read(&sk->sk_wmem_alloc));

	if (!sock_flag(sk, SOCK_DEAD)) {
		pr_err("Attempt to release alive stp socket %p\n", sk);
		return;
	}

	sk_refcnt_debug_dec(sk);
}


/* Create an STP socket */
static int stp_create(struct net *net, struct socket *sock,
		int protocol, int kern)
{
	struct sock *sk;
	struct stp_sock *ssk;

	if (sock->type != SOCK_DGRAM || protocol != 0)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	sk = sk_alloc(net, PF_STP, GFP_KERNEL, &stp_proto);
	if (!sk) {
		printk(KERN_ALERT "Error allocating sk\n");
		return -ENOBUFS;
	}

	sock->ops = &stp_ops;
	sock_init_data(sock, sk);

	ssk = (struct stp_sock*) sk;

	sk->sk_family = PF_STP;

	sk->sk_destruct = stp_sock_destruct;
	sk_refcnt_debug_inc(sk);

	spin_lock_init(&ssk->lock);
	ssk->bnd_ifindex = 0;
	ssk->bnd_port = 0;

	return 0;
}


static const struct net_proto_family stp_family_ops = {
	.family	= PF_STP,
	.create = stp_create,
	.owner	= THIS_MODULE,
};

static int stp_seq_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "%10s %10s %10s %10s %10s %10s\n",
		"RxPkts", "HdrErr", "CsumErr",
		"NoSock", "NoBuffs", "TxPkts");

	seq_printf(seq, "%10d %10d %10d %10d %10d %10d\n",
		stp_stats.num_rx_pkts,
		stp_stats.num_hdr_err,
		stp_stats.num_csum_err,
		stp_stats.num_no_sock,
		stp_stats.num_no_buffs,
		stp_stats.num_tx_pkts);
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
		printk(KERN_ALERT "Unable to create /proc/net/ entry");
		goto out;
	}

	ret = proto_register(&stp_proto, 0);
	if (ret != 0) {
		printk(KERN_ALERT "Unable to register protocol\n");
		goto out_clear_proc;
	}

	ret = sock_register(&stp_family_ops);
	if (ret != 0) {
		printk(KERN_ALERT "Unable to register STP protocol ops\n");
		goto out_proto_unregister;
	}

	dev_add_pack(&stp_packet_type);

	return 0;

out_proto_unregister:
	proto_unregister(&stp_proto);
out_clear_proc:
	unregister_pernet_subsys(&stp_proc_ops);
out:
	return ret;
}

void __exit stp_exit(void)
{
	dev_remove_pack(&stp_packet_type);
	sock_unregister(PF_STP);
	proto_unregister(&stp_proto);
	unregister_pernet_subsys(&stp_proc_ops);

	stp_bind_socks_clear();
}

module_init(stp_init);
module_exit(stp_exit);


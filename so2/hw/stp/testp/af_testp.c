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

#define AF_TESTP 19
#define PF_TESTP 19

/* Simple module for testing the addition of a new protocol family */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TESTP");
MODULE_AUTHOR("Dragos Tarcatu");

MODULE_ALIAS_NETPROTO(PF_TESTP);

struct testp_sock {
	struct sock *sk;
	int some_other_stuff;
	spinlock_t lock;
};

static inline struct testp_sock * testp_sk(struct sock *sk)
{
	return (struct testp_sock *) sk;
}

static struct proto testp_proto = {
	.name = "testp",
	.owner = THIS_MODULE,
	.obj_size = sizeof(struct testp_sock),
};

static int testp_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct testp_sock *ts;
	struct net *net;

	if (!sk)
		return 0;

	//skb_queue_purge(&sk->sk_receive_queue);
	//__sock_put(sk);

	printk(KERN_ALERT "releasing socket: %p sock: %p refcnt: %d wmem: %d rmem: %d\n",
		sock, sk,
		atomic_read(&sk->sk_refcnt),
		atomic_read(&sk->sk_wmem_alloc),
		atomic_read(&sk->sk_rmem_alloc));

	net = sock_net(sk);
	ts = testp_sk(sk);

	synchronize_net();
	sock_orphan(sk);
	sock->sk = NULL;

	sk_refcnt_debug_release(sk);
	sock_put(sk);

	return 0;
}


int testp_sendmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t size)
{
	printk(KERN_ALERT "testp_sendmsg socket: %p\n", sock);
	return 0;
}

int testp_recvmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *msg,
		 size_t size, int flags)
{
	printk(KERN_ALERT "testp_recvmsg socket: %p\n", sock);
	return 0;
}

static const struct proto_ops testp_ops = {
	.family		= PF_TESTP,
	.owner		= THIS_MODULE,
	.release	= testp_release,
	.bind		= sock_no_bind,
	.connect	= sock_no_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= sock_no_accept,
	.getname	= sock_no_getname,
	.poll		= sock_no_poll,
	.ioctl		= sock_no_ioctl,
	.listen		= sock_no_listen,
	.shutdown	= sock_no_shutdown,
	.setsockopt	= sock_no_setsockopt,
	.getsockopt	= sock_no_getsockopt,
	.sendmsg	= testp_sendmsg,
	.recvmsg	= testp_recvmsg,
	.mmap		= sock_no_mmap,
	.sendpage	= sock_no_sendpage,
};

static void testp_sock_destruct(struct sock *sk)
{
	printk(KERN_ALERT "destructing sock: %p refcnt: %d wmem: %d rmem: %d\n",
		sk,
		atomic_read(&sk->sk_refcnt),
		atomic_read(&sk->sk_wmem_alloc),
		atomic_read(&sk->sk_rmem_alloc));

	skb_queue_purge(&sk->sk_error_queue);
	skb_queue_purge(&sk->sk_receive_queue);

	WARN_ON(atomic_read(&sk->sk_rmem_alloc));
	WARN_ON(atomic_read(&sk->sk_wmem_alloc));

	if (!sock_flag(sk, SOCK_DEAD)) {
		pr_err("Attempt to release alive testp socket %p\n", sk);
		return;
	}

	sk_refcnt_debug_dec(sk);
}

static int testp_create(struct net *net, struct socket *sock,
			int protocol, int kern)
{
	struct sock *sk;


	sk = sk_alloc(net, PF_TESTP, GFP_KERNEL, &testp_proto);
	if (!sk) {
		printk(KERN_ALERT "Unable to allocate sk\n");
		return -ENOBUFS;
	}

	sock_init_data(sock, sk);

	printk(KERN_ALERT "creating socket: %p sock: %p refcnt: %d wmem: %d rmem: %d\n",
		sock, sk,
		atomic_read(&sk->sk_refcnt),
		atomic_read(&sk->sk_wmem_alloc),
		atomic_read(&sk->sk_rmem_alloc));


	sk->sk_family = PF_TESTP;
	sk->sk_protocol = protocol;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;

	sk->sk_destruct = testp_sock_destruct;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;
	sk_refcnt_debug_inc(sk);
	spin_lock_init(&testp_sk(sk)->lock);

	//sock->state = SS_UNCONNECTED;
	sock->ops = &testp_ops;

	return 0;
}

static const struct net_proto_family testp_family_ops = {
	.family	= PF_TESTP,
	.create = testp_create,
	.owner	= THIS_MODULE,
};


int __init testp_init(void)
{
	int ret;

	ret = proto_register(&testp_proto, 0);
	if (ret < 0) {
		printk(KERN_ALERT "Unable to register protocol\n");
	}

	sock_register(&testp_family_ops);

	return 0;
}

void __exit testp_exit(void)
{
	sock_unregister(PF_TESTP);
	proto_unregister(&testp_proto);
}

module_init(testp_init);
module_exit(testp_exit);

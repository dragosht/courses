#ifndef TRTEST_H__
#define TRTEST_H__


#include <asm/ioctl.h>


#define TRTEST_KMALLOC		_IOC(_IOC_WRITE, 't', 1, sizeof(int))
#define TRTEST_KFREE		_IOC(_IOC_NONE, 't', 1, 0)

#endif


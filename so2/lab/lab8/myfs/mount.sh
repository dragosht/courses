#!/bin/sh

#load module
insmod myfs.ko

#mount filesystem
mkdir -p /mnt/myfs
mount -t myfs none /mnt/myfs


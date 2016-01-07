#!/bin/sh

set -x

#load module
insmod minfs.ko

#create mount point
mkdir -p /mnt/minfs

#format partition
./mkfs.minfs /dev/sdb

#mount filesystem
mount -t minfs /dev/sdb /mnt/minfs



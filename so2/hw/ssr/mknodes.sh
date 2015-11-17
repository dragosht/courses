#!/bin/sh

mknod /dev/ssr b 240 0
dd if=/dev/zero of=/dev/sda bs=8M
dd if=/dev/zero of=/dev/sdb bs=8M

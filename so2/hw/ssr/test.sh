#!/bin/sh

mknod /dev/ssr b 240 0

insmod ssr.ko
echo "test" > /dev/ssr


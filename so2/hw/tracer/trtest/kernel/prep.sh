#!/bin/sh

mknod /dev/tracer c 10 42
mknod /dev/trtest c 42 0
insmod tracer.ko
insmod trtest.ko

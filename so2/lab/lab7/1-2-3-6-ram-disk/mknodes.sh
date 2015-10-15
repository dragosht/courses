#!/bin/sh

mknod /dev/myblock b 240 0

# To generate write requests ...
#echo "abc" > /dev/myblock

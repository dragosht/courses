#!/bin/sh

#ssr /proc/modules 0xc8887000
#EIP - base addr: 

addr2line -e ssr.ko 0x48c

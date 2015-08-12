#!/bin/sh

#sudo su
#wlan0 is my internet facing interface, usb0 is the BeagleBone USB connection
ifconfig usb0 192.168.7.1
iptables --flush
iptables --table nat --flush
iptables --delete-chain
iptables --table nat --delete-chain
iptables --table nat --append POSTROUTING --out-interface wlan0 -j MASQUERADE
iptables --append FORWARD --in-interface usb0 -j ACCEPT
echo 1 > /proc/sys/net/ipv4/ip_forward

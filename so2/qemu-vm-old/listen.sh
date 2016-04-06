#!/bin/sh

#need to listen on tap1 172.30.0.1

#netcat -lup 6666

#netcat -u 172.20.0.1 6666

netcat -lu 172.20.0.1 6666

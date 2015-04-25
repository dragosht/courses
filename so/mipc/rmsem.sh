#!/bin/sh

#shows opened posix semaphores - kinda like ipcs -s (Sys V)
#ls -al /dev/shm/sem.*


sudo rm -f /dev/shm/sem.mipc-sem
sudo rm -f /dev/shm/shm.mipc-shm
